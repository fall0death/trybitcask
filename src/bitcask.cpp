


#include "bitcask.h"


namespace bitcask{

Bitcask::Bitcask(){
    active_data_id = 0;
    active_hint_id = 0;
}

Bitcask::~Bitcask(){
    this->close();
}

bool close(){
    if(){
        
    }
}

void init(){
    try{
        init_directory(data_directory);
        init_directory(hint_directory);

        init_file(flag);

    }catch(error e){
        throw e;
    }
    


}

void init_file(const std::string & directory , const std::string & file){
    DIR *dir;  
    struct dirent *d;
    std::string s = directory + "/" + file;  
    if((dir=opendir(s.c_str()))==NULL){  
        error e(2,directory+" not found");  
        throw e;
    } else {  
        while((entry=readdir(directory_pointer))!=NULL){  
            if(entry->d_name[0]=='.') continue;  
            num++;
        }  
    } 
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