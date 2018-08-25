

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
    for (uint64_t i = 0; i <= active_hint_id; i++)
    {
        std::ifstream hint_file;
        std::string path = hint_directory + "/" + hint_file_name + int_to_string(i);
        if(file_is_exist(path)){
            continue;
        }
        hint_file.open(path.data(), std::ios::binary | std::ios::in);
        if (!hint_file.is_open())
        {
            error e(4, path + "文件打开失败");
            throw e;
        }
        hint_file.seekg(0, std::ios::beg);


        while (hint_file.peek()!=EOF)
        {
            struct bitcask_index index;
            hint_file.read((char *)&(index.time), sizeof(time_t));
            hint_file.read((char *)&(index.flag), sizeof(bool));
            hint_file.read((char *)&(index.key_len), sizeof(uint64_t));
            char *key = new char[index.key_len];
            hint_file.read(key, index.key_len);
            hint_file.read((char *)&(index.file_id), sizeof(uint64_t));
            hint_file.read((char *)&(index.file_pos), sizeof(uint32_t));
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
        hash_lock.lock();
        bitcask_data b_d;
        time(&b_d.time);
        b_d.key_len = key.length()+1;
        b_d.key = key;
        b_d.value_len = value.length()+1;
        b_d.value = value;
        bitcask_index b_i;
        b_i.file_pos=write_data_file(b_d);
        b_i.time=b_d.time;
        b_i.flag = true;
        b_i.key_len = key.length()+1;
        b_i.key = key;
        b_i.file_id = active_data_id;
        write_hint_file(b_i);
        index_hash[key] = b_i;
        hash_lock.unlock();
    }catch(error e){
        hash_lock.unlock();
        throw e;
    }
} //增操作
void Bitcask::_update(const std::string &key,const std::string &value){
    try{
        int i = index_hash.count(key);
        if(i){
            bitcask_index b_i = index_hash[key];
            if(b_i.flag){
                hash_lock.lock();
                bitcask_data b_d;
                time(&b_d.time);
                b_d.key_len = key.length()+1;
                b_d.key = key;
                b_d.value_len = value.length()+1;
                b_d.value = value;
                b_i.file_pos=write_data_file(b_d);
                b_i.time=b_d.time;
                b_i.flag = true;
                b_i.key_len = key.length()+1;
                b_i.key = key;
                b_i.file_id = active_data_id;
                write_hint_file(b_i);
                index_hash[key] = b_i;
                hash_lock.unlock();
                return;
            }
        }
        hash_lock.unlock();
        error e(1,"字段\""+key+"\"不存在");
        throw e;
    }catch(error e){
        hash_lock.unlock();
        throw e;
    }
} //改操作
void Bitcask::_delete(const std::string &key){
    try{
        int i = index_hash.count(key);
        if(i){
            hash_lock.lock();
            bitcask_index b_i = index_hash[key];
            if(b_i.flag){
                b_i.flag = false;
                write_hint_file(b_i);
                index_hash[key] = b_i;
                hash_lock.unlock();
                return;
            }
            hash_lock.unlock();
        }
        error e(1,"字段\""+key+"\"不存在");
        throw e;
    }catch(error e){
        hash_lock.unlock();
        throw e;
    }
} //删操作
void Bitcask::_select(const std::string& key,time_t &t,std::string &value){
    try{
        int i = index_hash.count(key);
        if(i){
            bitcask_index b_i = index_hash[key];
            if(b_i.flag){
                read_lock.lock();

                std::string path = data_directory + "/" + data_file_name + int_to_string(b_i.file_id);
                std::string merge_path = path+merge_name;
                if(!file_is_exist(merge_path)){
                    if(!read_data_file(merge_path,b_i.file_pos,b_i.key,value,t)){
                        if(!read_data_file(path,b_i.file_pos,b_i.key,value,t)){
                            read_lock.unlock();
                            error e(3,path+"数据出现问题！");
                            throw e;
                        }
                    }
                }else{
                    if(!read_data_file(path,b_i.file_pos,b_i.key,value,t)){
                        read_lock.unlock();
                        error e(3,path+"数据出现问题！");
                        throw e;
                    }
                }//如果是正在merge时查询，有可能哈希表被更新，所以需要进行两次判断
                

                read_lock.unlock();
                return;
            }
        }
        error e(1,"字段\""+key+"\"不存在");
        throw e;
    }catch(error e){
        throw e;
    }
} //查操作
bool Bitcask::read_data_file(const std::string& path,const uint32_t& file_pos,const std::string &key,std::string& value,time_t& t){
    std::ifstream data_file(path.data(),std::ios::in|std::ios::binary);
    if(!data_file.is_open()){
        return false;
        //error e(3,"打开文件\""+path+"\"失败！");
        //throw e;
    }
    uint32_t size = file_size(path);
    if(file_pos>=size){
        return false;
        //error e(3,"数据出现问题！");
        //throw e;
    }
    data_file.seekg(file_pos,std::ios::beg);

    uint64_t key_len;
    uint64_t value_len;
    data_file.read((char *)&t, sizeof(time_t));
    data_file.read((char *)&key_len,sizeof(uint64_t));
    char* key_ = new char[key_len];
    data_file.read(key_,key_len);
    std::string key_s = key;
    if(key!=key_s){
        return false;
    }
    data_file.read((char *)&value_len,sizeof(uint64_t));
    char* value_ = new char[value_len];
    data_file.read(value_,value_len);
    value = value_;
    return true;
}
void Bitcask::_merge()
{
    uint64_t new_hint_id = 0;
    uint32_t new_hint_pos = 0;
    std::string  path = hint_directory + "/" + hint_file_name + int_to_string(new_hint_id)+merge_name;
    std::ofstream new_hint_file(path.c_str(),std::ios::binary | std::ios::out | std::ios::app);
    try{
        new_hint_pos = file_size(path);
    }catch(error e){
        throw e;
    }//打开新的hint文件用来merge

    uint64_t new_data_id = 0;
    uint32_t new_data_pos = 0;
    path = data_directory + "/" + data_file_name + int_to_string(new_data_id)+merge_name;
    std::ofstream new_data_file(path.c_str(),std::ios::binary | std::ios::out | std::ios::app);
    try{
        new_data_pos = file_size(path);
    }catch(error e){
        throw e;
    }//打开新的data文件用来merge

    for (uint64_t i = 0; i < active_hint_id; i++)
    {
        std::ifstream hint_file;
        std::string path = hint_directory + "/" + hint_file_name + int_to_string(i);
        if(file_is_exist(path)){
            continue;
        }//判断索引文件在不在
        hint_file.open(path.data(), std::ios::binary | std::ios::in);
        if (!hint_file.is_open())
        {
            error e(4, path + "文件打开失败");
            throw e;
        }
        hint_file.seekg(0, std::ios::beg);


        while (hint_file.peek()!=EOF)
        {
            struct bitcask_index index;
            hint_file.read((char *)&(index.time), sizeof(time_t));
            hint_file.read((char *)&(index.flag), sizeof(bool));
            hint_file.read((char *)&(index.key_len), sizeof(uint64_t));
            char *key = new char[index.key_len];
            hint_file.read(key, index.key_len);
            hint_file.read((char *)&(index.file_id), sizeof(uint64_t));
            hint_file.read((char *)&(index.file_pos), sizeof(uint32_t));
            index.key = key;//读出旧数据

            hash_lock.lock();//更改哈希表前获取互斥锁,防止更改索引时把新的索引覆盖了


            if(!index_hash.count(index.key)){
                continue;
            }
            if(!index_hash[index.key].flag){
                continue;
            }
            if(index_hash[index.key].file_id>index.file_id){
                continue;
            }
            if(index_hash[index.key].file_id==index.file_id&&index_hash[index.key].file_pos>index.file_pos){
                continue;
            }//若索引为旧的，则不做处理（舍去）
            
            try{
                if(index_hash[index.key].file_id==active_data_id){//若该索引指向的数据在最新的文件中，则更新索引即可
                    merge_hint_file(new_hint_id,new_hint_pos,new_hint_file,index_hash[index.key]);
                }else{
                    path = data_directory + "/" + data_file_name + int_to_string(index.file_id);
                    std::ifstream data_file(path.data(),std::ios::in|std::ios::binary);
                    if(!data_file.is_open()){
                        error e(3,"打开文件\""+path+"\"失败！");
                        throw e;
                    }
                    uint32_t size = file_size(path);
                    if(index.file_pos>=size){
                        error e(3,"数据出现问题！");
                        throw e;
                    }
                    data_file.seekg(index.file_pos,std::ios::beg);

                    struct bitcask_data b_d;
                    data_file.read((char *)&b_d.time, sizeof(time_t));
                    data_file.read((char *)&b_d.key_len,sizeof(uint64_t));
                    char* key_ = new char[b_d.key_len];
                    data_file.read(key_,b_d.key_len);
                    b_d.key = key_;
                    data_file.read((char *)&b_d.value_len,sizeof(uint64_t));
                    char* value_ = new char[b_d.value_len];
                    data_file.read(value_,b_d.value_len);
                    b_d.value = value_;//拿到数据文件

                    index.file_pos = merge_data_file(new_data_id,new_data_pos,new_data_file,b_d);//整合进新数据文件
                    index.file_id = new_data_id;
                    merge_hint_file(new_hint_id,new_hint_pos,new_hint_file,index);//整合进新索引文件

                    index_hash[index.key] = index;

                    hash_lock.unlock();
                }
            }catch(error e){
                throw e;
            }
            
        }
    }
    for(uint64_t i=0;i<active_hint_id;i++){
        path = hint_directory + "/" + hint_file_name + int_to_string(i);  
        if(!file_is_exist(path)){
            continue;
        }else{
            remove(path.c_str());
        }
        if(i<=new_hint_id){
            std::string merge_path = path + merge_name;
            if(rename(merge_path.c_str(),path.c_str())){
                error e(1,merge_path+"更名失败！");
                read_lock.unlock();
                throw e;
            }
        }
    }

    read_lock.lock();//加锁防止查时找不到数据文件

    for(uint64_t i=0;i<active_data_id;i++){
        path = data_directory + "/" + data_file_name + int_to_string(i);  
        if(!file_is_exist(path)){
            continue;
        }else{
            remove(path.c_str());
        }
        if(i<=new_data_id){
            std::string merge_path = path + merge_name;
            if(rename(merge_path.c_str(),path.c_str())){
                error e(1,merge_path+"更名失败！");
                read_lock.unlock();
                throw e;
            }
        }
    }

    read_lock.unlock();
} //整合数据

uint64_t Bitcask::merge_data_file(uint64_t& data_id,uint32_t &data_pos,std::ofstream& data_ostream,const bitcask_data& b_d){
    uint32_t pos = data_pos;
    data_pos += sizeof(time_t)+sizeof(uint64_t)+sizeof(uint64_t)+b_d.key_len+b_d.value_len;
    if(active_data_pos>=max_file){
        data_ostream.close();
        data_id++;
        std::string path = data_directory + "/" + data_file_name + int_to_string(data_id)+merge_name;
        data_ostream.open(path.c_str(), std::ios::binary | std::ios::out | std::ios::app);
        try{
            data_pos = file_size(path);
        }catch(error e){
            throw e;
        }
        
        return merge_data_file(data_id,data_pos,data_ostream,b_d); 
    }

    data_ostream.write((char*)&(b_d.time), sizeof(time_t));
    data_ostream.write((char*)&(b_d.key_len), sizeof(uint64_t));
    data_ostream.write(b_d.key.data(),b_d.key_len);
    data_ostream.write((char*)&(b_d.value_len), sizeof(uint64_t));
    data_ostream.write(b_d.value.data(),b_d.value_len);

    data_ostream.flush();

    return pos;
    
}//整合数据文件

void Bitcask::merge_hint_file(uint64_t& hint_id,uint32_t &hint_pos,std::ofstream& hint_ostream,const bitcask_index& b_i){
    uint32_t pos = hint_pos;
    hint_pos += sizeof(time_t)+sizeof(bool)+sizeof(uint64_t)+b_i.key_len+sizeof(uint64_t)+sizeof(uint32_t);
    if(hint_pos>=max_file){
        hint_ostream.close();
        hint_id++;
        std::string path = hint_directory + "/" + hint_file_name + int_to_string(hint_id)+merge_name;
        hint_ostream.open(path.c_str(), std::ios::binary | std::ios::out | std::ios::app);
        
        
        try{
            hint_pos = file_size(path);
        }catch(error e){
            throw e;
        }

        merge_hint_file(hint_id,hint_pos,hint_ostream,b_i); 
        return;
    }

    hint_ostream.write((char*)&(b_i.time), sizeof(time_t));
    hint_ostream.write((char*)&(b_i.flag), sizeof(bool));
    hint_ostream.write((char*)&(b_i.key_len), sizeof(uint64_t));
    hint_ostream.write(b_i.key.data(),b_i.key_len);
    hint_ostream.write((char*)&(b_i.file_id), sizeof(uint64_t));
    hint_ostream.write((char*)&(b_i.file_pos), sizeof(uint32_t));

    hint_ostream.flush();

    return;
    
}//整合索引文件

uint64_t Bitcask::write_data_file(const bitcask_data& b_d){
    uint32_t pos = active_data_pos;
    active_data_pos += sizeof(time_t)+sizeof(uint64_t)+sizeof(uint64_t)+b_d.key_len+b_d.value_len;
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
    active_data_file.write(b_d.key.data(),b_d.key_len);
    active_data_file.write((char*)&(b_d.value_len), sizeof(uint64_t));
    active_data_file.write(b_d.value.data(),b_d.value_len);

    active_data_file.flush();

    return pos;
    
}//写入数据文件

void Bitcask::write_hint_file(const bitcask_index& b_i){
    uint32_t pos = active_hint_pos;
    active_hint_pos += sizeof(time_t)+sizeof(bool)+sizeof(uint64_t)+b_i.key_len+sizeof(uint64_t)+sizeof(uint64_t);
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
    active_hint_file.write(b_i.key.data(),b_i.key_len);
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