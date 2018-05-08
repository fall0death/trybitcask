

#include "bitcask.h"

namespace bitcask
{

Bitcask::Bitcask()
{
    this->active_data_id = 0;
    this->active_hint_id = 0;
}

Bitcask::~Bitcask()
{ 
    this->close();
}

void Bitcask::close()
{
    if (active_data_file.is_open())
    {
        active_data_file.close();
    }
    if (active_hint_file.is_open())
    {
        active_hint_file.close();
    }
    Bitcask::index_hash.clear();
}

void Bitcask::init()
{
    try
    {
        this->init_directory(data_directory);
        this->init_directory(hint_directory);

        this->active_data_id = this->find_max_id(data_directory, data_file_name);
        this->active_hint_id = this->find_max_id(hint_directory, hint_file_name);

        this->file_ostream();

        this->map_create();
    }
    catch (error e)
    {
        throw e;
    }
}

// ios::in	为输入(读)而打开文件
// ios::out	为输出(写)而打开文件
// ios::ate	初始位置：文件尾
// ios::app	所有输出附加在文件末尾
// ios::trunc	如果文件已存在则先删除该文件
// ios::binary	二进制方式

std::string int_to_string(uint64_t i){
    std::stringstream ss;
    ss<<i;
    std::string str;
    ss>>str;
    return str;
}

void Bitcask::file_ostream()
{
    std::string path = data_directory + "/" + data_file_name + int_to_string(active_data_id);
    active_data_file.open(path.c_str(), std::ios::binary | std::ios::out | std::ios::app);
    if (!active_data_file.is_open())
    {
        error e(4, path + "文件打开失败");
        throw e;
    } //打开文件，没有的话自动创建

    try{
        active_data_pos = this->file_size(path);
    }catch(error e){
        throw e;
    }
    



    path = hint_directory + "/" + hint_file_name + int_to_string(active_hint_id);
    active_hint_file.open(path.data(), std::ios::binary | std::ios::out | std::ios::app);
    if (!active_hint_file.is_open())
    {
        error e(4, path + "文件打开失败");
        throw e;
    }

    try{
        active_hint_pos = this->file_size(path);
    }catch(error e){
        throw e;
    }
}

// struct dirent     在 dirent.h头文件中
// {
// 　　long d_ino; /* inode number 索引节点号 */
//     off_t d_off; /* offset to this dirent 在目录文件中的偏移 */
//     unsigned short d_reclen; /* length of this d_name 文件名长 */
//     unsigned char d_type; /* the type of d_name 文件类型 */
//     char d_name [NAME_MAX+1]; /* file name (null-terminated) 文件名，最长255字符 */
// }

int Bitcask::find_max_id(const std::string &directory, const std::string &file)
{
    DIR *dir;
    struct dirent *d;
    int max = 0;
    if ((dir = opendir(directory.c_str())) == NULL)
    {
        error e(2, directory + " not found");
        throw e;
    }
    else
    {
        while ((d = readdir(dir)) != NULL)
        {
            //if(d->d_name[0]=='.') continue;
            int i = 0;
            for (i = 0; i < file.length() && i < d->d_reclen; i++)
            {
                if (d->d_name[i] != file[i])
                {
                    break;
                }
            }
            if (i != file.length())
                continue;
            //判断是否有对应文件名的前缀
            int num = 0;
            for (; i < d->d_reclen; i++)
            {
                if (d->d_name[i] >= '0' && d->d_name[i] <= '9')
                {
                    num = num * 10 + (d->d_name[i] - '0');
                }
                else
                {
                    break;
                    num = 0;
                }
            } //拿到当前文件的id
            if (num != 0)
            {
                if (max < num)
                    max = num;
            }
        }
    }
    closedir(dir);
    return max;
}

void Bitcask::init_directory(const std::string &directory)
{
    int suc = file_is_exist(directory);
    if (suc == 1)
    {
        error e(1, directory + "目录字符串丢失");
        throw e;
    }
    if (suc == -1)
    {
        int status = new_folder(directory);
        if (status == -1)
        {
            error e(1, directory + "目录字符串丢失");
            throw e;
        }
    }
}

int Bitcask::new_folder(const std::string &s)
{
    return mkdir(s.data(),0x700);
}

int Bitcask::file_is_exist(const std::string &s)
{
    if (s.empty())
    {
        return 1;
    }
    else
    {
        return access(s.c_str(), 06);
        //这里也可以用s.data()，但这里是const char* ，如果要变成char*要强制类型转换
    }
}

void Bitcask::map_create()
{
    for (uint64_t i = 0; i < active_hint_id; i++)
    {
        std::ifstream hint_file;
        std::string path = hint_directory + "/" + hint_file_name + int_to_string(i);
        hint_file.open(path.data(), std::ios::binary | std::ios::in);
        if (!hint_file.is_open())
        {
            error e(4, path + "文件打开失败");
            throw e;
        }
        hint_file.seekg(0, std::ios::beg);

        Bitcask::index_hash.reserve(INT_MAX);
        Bitcask::index_hash.rehash(INT_MAX);

        while (!hint_file.eof())
        {
            struct bitcask_index index;
            hint_file.read((char *)&(index.time), sizeof(time_t));
            hint_file.read((char *)&(index.flag), sizeof(bool));
            hint_file.read((char *)&(index.key_len), sizeof(uint64_t));
            char *key = new char[index.key_len];
            hint_file.read(key, index.key_len);
            hint_file.read((char *)&(index.file_id), sizeof(uint64_t));
            hint_file.read((char *)&(index.file_pos), sizeof(uint64_t));
            index.key = key;

            Bitcask::index_hash[index.key] = index;
        }
    }
}

// struct bitcask_index{
//     time_t time;//增加数据的时间
//     bool flag;//是否已被删除
//     uint64_t key_len;//key的长度
//     std::string key;//数据的索引
//     uint64_t file_id;//数据所在位置文件的id
//     uint64_t file_pos;//数据所在文件的位置
// };//存储数据位置的结构体
// struct bitcask_data{
//     time_t time;//增加数据的时间
//     uint64_t key_len;//key的长度
//     std::string key;//数据的索引
//     uint64_t value_len;//数据段的长度
//     std::string value;//数据
// };//存储数据的结构体
void Bitcask::_insert(const std::string& key,const std::string& value)
{
    try{
        int i = index_hash.count(key);
        if(i){
            bitcask_index b_i = index_hash[key];
            if(b_i.flag){
                error e(4,"字段\""+key+"\"已经存在！");
                throw e;
            }
        }
        bitcask_data b_d;
        b_d.time = time(NULL);
        b_d.key_len = key.length();
        b_d.key = key;
        b_d.value_len = value.length();
        b_d.value = value;
        bitcask_index b_i;
        b_i.file_pos=write_data_file(b_d);
        b_i.time = time(NULL);
        b_i.flag = true;
        b_i.key_len = key.length();
        b_i.key = key;
        b_i.file_id = active_data_id;
        write_hint_file(b_i);
    }catch(error e){
        throw e;
    }
} //增操作
void Bitcask::_update(const std::string &key,const std::string &value){
    try{
        int i = index_hash.count(key);
        if(i){
            bitcask_index b_i = index_hash[key];
            if(b_i.flag){
                bitcask_data b_d;
                b_d.time = time(NULL);
                b_d.key_len = key.length();
                b_d.key = key;
                b_d.value_len = value.length();
                b_d.value = value;
                b_i.file_pos=write_data_file(b_d);
                b_i.time = time(NULL);
                b_i.flag = true;
                b_i.key_len = key.length();
                b_i.key = key;
                b_i.file_id = active_data_id;
                write_hint_file(b_i);
                return;
            }
        }
        error e(1,"字段\""+key+"\"不存在");
        throw e;
    }catch(error e){
        throw e;
    }
} //改操作
void Bitcask::_delete(const std::string &key){
    try{
        int i = index_hash.count(key);
        if(i){
            bitcask_index b_i = index_hash[key];
            if(b_i.flag){
                b_i.flag = false;
                write_hint_file(b_i);
                return;
            }
        }
        error e(1,"字段\""+key+"\"不存在");
        throw e;
    }catch(error e){
        throw e;
    }
} //删操作
void Bitcask::_select(const std::string& key,time_t &t,std::string &value){
    try{
        int i = index_hash.count(key);
        if(i){
            bitcask_index b_i = index_hash[key];
            if(b_i.flag){
                std::string path = data_directory + data_file_name + int_to_string(b_i.file_id);
                std::ifstream data_file(path.data(),std::ios::in|std::ios::binary);
                if(!data_file.is_open()){
                    error e(3,"打开文件\""+path+"\"失败！");
                    throw e;
                }
                uint32_t size = file_size(path);
                if(b_i.file_pos>=size){
                    error e(3,"数据出现问题！");
                    throw e;
                }
                data_file.seekg(b_i.file_pos,std::ios::beg);

                uint64_t key_len;
                uint64_t value_len;
                data_file.read((char *)&t, sizeof(time_t));
                data_file.read((char *)&key_len,sizeof(uint64_t));
                char* key_ = new char[key_len];
                data_file.read(key_,key_len);
                data_file.read((char *)&value_len,sizeof(uint64_t));
                char* value_ = new char[value_len];
                data_file.read(value_,value_len);
                value = value_;
            }
        }
        error e(1,"字段\""+key+"\"不存在");
        throw e;
    }catch(error e){
        throw e;
    }
} //查操作
void Bitcask::_merge()
{
    
} //整合数据

uint64_t Bitcask::write_data_file(const bitcask_data& b_d){
    uint32_t pos = active_data_pos;
    active_data_pos += sizeof(time_t)+sizeof(uint64_t)+sizeof(uint64_t)+b_d.key.length()+b_d.value.length();
    if(active_data_pos>=max_file){
        active_data_file.close();
        active_data_id++;
        std::string path = data_directory + "/" + data_file_name + int_to_string(active_data_id);
        active_data_file.open(path.c_str(), std::ios::binary | std::ios::out | std::ios::app);
        try{
            active_data_pos = file_size(path);
        }catch(error e){
            throw e;
        }
        
        return write_data_file(b_d); 
    }

    active_data_file.write((char*)&(b_d.time), sizeof(time_t));
    active_data_file.write((char*)&(b_d.key_len), sizeof(uint64_t));
    active_data_file.write(b_d.key.data(),b_d.key.length());
    active_data_file.write((char*)&(b_d.value_len), sizeof(uint64_t));
    active_data_file.write(b_d.value.data(),b_d.value.length());

    active_data_file.flush();

    return pos;
    
}//写入数据文件

void Bitcask::write_hint_file(const bitcask_index& b_i){
    uint32_t pos = active_hint_pos;
    active_hint_pos += sizeof(time_t)+sizeof(bool)+sizeof(uint64_t)+b_i.key.length()+sizeof(uint64_t)+sizeof(uint64_t);
    if(active_hint_pos>=max_file){
        active_hint_file.close();
        active_hint_id++;
        std::string path = hint_directory + "/" + hint_file_name + int_to_string(active_hint_id);
        active_hint_file.open(path.c_str(), std::ios::binary | std::ios::out | std::ios::app);
        
        
        try{
            active_hint_pos = file_size(path);
        }catch(error e){
            throw e;
        }

        write_hint_file(b_i); 
        return;
    }

    active_hint_file.write((char*)&(b_i.time), sizeof(time_t));
    active_hint_file.write((char*)&(b_i.flag), sizeof(bool));
    active_hint_file.write((char*)&(b_i.key_len), sizeof(uint64_t));
    active_hint_file.write(b_i.key.data(),b_i.key.length());
    active_hint_file.write((char*)&(b_i.file_id), sizeof(uint64_t));
    active_hint_file.write((char*)&(b_i.file_pos), sizeof(uint32_t));

    active_hint_file.flush();

    return;
    
}//写入索引文件

uint32_t Bitcask::file_size(const std::string &path){
    struct stat fileInfo;
        if (stat(path.c_str(), &fileInfo) < 0){
            error e(1,"文件"+path+"打开失败！");
            throw e;
        }
    return fileInfo.st_size;
}

error::error(short i,std::string msg){
    this->i = i;
    this->msg = msg;
}

std::string error::toString(){
    return msg;
}

}