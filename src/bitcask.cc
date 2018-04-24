


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

        active_data_id =find_max_id(data_directory,data_file_name);
        active_hint_id = find_max_id(hint_directory,hint_file_name);

        

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

int find_max_id(const std::string & directory , const std::string & file){
    DIR *dir;  
    struct dirent *d;
    int max = -1;
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
            int num = -1;
            for(;i<d->d_reclen;i++){
                if(d->d_name[i]>='0'&&d->d_name[i]<='9'){
                    if(num == -1){
                        num = 0;
                    }
                    num = num * 10 + (d->d_name - '0');
                }else{
                    break;
                    num = -1;
                }
            }//拿到当前文件的id
            if(num!= -1){
                if(max == -1 || max < num) max = num;
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