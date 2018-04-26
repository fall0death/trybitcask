


#include "bitcask.h"


namespace bitcask{

Bitcask::Bitcask(){
    active_data_id = 0;
    active_hint_id = 0;
}

Bitcask::~Bitcask(){
    this->close();
}

void close(){
    if(active_data_file.is_open()){
        active_data_file.close();
    }
    if(active_hint_file.is_open()){
        active_hint_file.close();
    }
    index_hash.clear();
}

void init(){
    try{
        this->init_directory(data_directory);
        this->init_directory(hint_directory);

        this->active_data_id = this->find_max_id(data_directory,data_file_name);
        this->active_hint_id = this->find_max_id(hint_directory,hint_file_name);

        this->file_ostream();
        
        this->map_create();
        
        

    }catch(error e){
        throw e;
    }
}

// ios::in	为输入(读)而打开文件
// ios::out	为输出(写)而打开文件
// ios::ate	初始位置：文件尾
// ios::app	所有输出附加在文件末尾
// ios::trunc	如果文件已存在则先删除该文件
// ios::binary	二进制方式

void file_ostream(){
    std::string path = data_directory + "/" + data_file_name + active_data_id ;
    active_data_file.open(path,ios::binary|ios::out|ios::app);
    if(!active_data_file.is_open()){
        error e(4,path+"文件打开失败");
        throw e;
    }//打开文件，没有的话自动创建
    path = hint_directory + "/" + hint_file_name + active_hint_id ;
    active_hint_file.open(path,ios::binary|ios::out|ios::app);
    if(!active_hint_file.is_open()){
        error e(4,path+"文件打开失败");
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

int find_max_id(const std::string & directory , const std::string & file){
    DIR *dir;  
    struct dirent *d;
    int max = 0;
    if((dir=opendir(directory.c_str()))==NULL){  
        error e(2,directory+" not found");  
        throw e;
    } else {
        while((d=readdir(dir))!=NULL){
            //if(d->d_name[0]=='.') continue;
            int i=0;
            for(i=0;i<file.length()&&i<d->d_reclen;i++){
                if(d->d_name[i]!=file[i]){
                    break;
                }
            }
            if(i!=file.length())continue;
            //判断是否有对应文件名的前缀
            int num = 0;
            for(;i<d->d_reclen;i++){
                if(d->d_name[i]>='0'&&d->d_name[i]<='9'){
                    num = num * 10 + (d->d_name - '0');
                }else{
                    break;
                    num = 0;
                }
            }//拿到当前文件的id
            if(num!= 0){
                if(max < num) max = num;
            }
        }
    }
    closedir(dir);
    return max;
}

void init_directory(const std::string &directory){
    int suc = file_is_exist(directory);
    if(suc == 1){
        error e(1,directory+"目录字符串丢失");
        throw e;
    }
    if(suc == -1) {
        int status = new_folder(directory);
        if(status == -1){
            error e(1,directory+"目录字符串丢失");
            throw e;
        }
    }
}

int new_folder(const std::string &s){
    std::string mkdir_ = "mkdir 700 "+s;
    return system(mkdir_.data());
}

int file_is_exist(const std::string s){
    if(s==nullptr){
        return 1;
    }else{
        return suc = access(s.c_str(),06);
        //这里也可以用s.data()，但这里是const char* ，如果要变成char*要强制类型转换
    }
}

void map_create(){
    for(int i = 0 ;i < active_hint_id;i++){
        ifstream hint_file;
        std::string path = data_directory + "/" + data_file_name + i ;
        hint_file.open(path,ios::binary|ios::in);
        if(!hint_file.is_open()){
            error e(4,path+"文件打开失败");
            throw e;
        }
        hint_file.seekg(0,ios::beg);

        index_hash.reserve(INT_MAX);
        index_hash.rehash(INT_MAX);

        while(!hint_file.iseof()){
            struct bitcask_index index;
            hint_file.read((char*)&(index.time),sizeof(time_t));
            hint_file.read((char*)&(index.flag),sizeof(bool));
            hint_file.read((char*)&(index.key_len),sizeof(int64_t));
            char* key = new char[index.key_len];
            hint_file.read(key,index.key_len);
            hint_file.read((char*)&(index.file_id),sizeof(int64_t));
            hint_file.read((char*)&(index.file_pos),sizeof(int64_t));
            index.key = key;

            index_hash.insert(make_pair<std::string,bitcask_index>(index.key,index));

        }
    }
}

bool _insert(){
    
}//增操作
bool _update(){
    
}//改操作
bool _delete(){
    
}//删操作
bool _select(){
    
}//查操作
bool _merge(){
    
}//整合数据

bool write_file(){
    
}
bool read_file(){
    
}



}