//小试牛刀，试试bitcask的源码编写

#ifndef _BITCASK_H
#define _BITCASK_H

#include<fstream>
#include<iostream>
#include<sstream>
#include<unordered_map>
#include<cstring>
#include<vector>
#include<time.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<dirent.h>
#include<limits.h>

namespace bitcask{

const std::string hint_directory = "hint";
const std::string data_directory = "data";
const std::string hint_file_name = "db_index_";
const std::string data_file_name = "db_data_";

struct bitcask_data{
    time_t time;//增加数据的时间
    uint64_t key_len;//key的长度
    std::string key;//数据的索引
    uint64_t value_len;//数据段的长度
    std::string value;//数据
};//存储数据的结构体

struct bitcask_index{
    time_t time;//增加数据的时间
    bool flag;//是否已被删除
    uint64_t key_len;//key的长度
    std::string key;//数据的索引
    uint64_t file_id;//数据所在位置文件的id
    uint32_t file_pos;//数据所在文件的位置
};//存储数据位置的结构体

class Bitcask{
    public:
        Bitcask();
        
        ~Bitcask();

        void close();//关闭输出流文件

        void init();//进行初始化操作

        void _insert(std::string key,std::string value);//增操作
        void _update(std::string key,std::string value);//改操作
        void _delete(std::string key);//删操作
        void _select(std::string key,std::string value);//查操作
        void _merge();//整合数据

        

    private:
        Bitcask(const Bitcask &b);
        void operator = (const Bitcask &b);

        std::ofstream active_data_file;
        std::ofstream active_hint_file;
        
        std::unordered_map<std::string,bitcask_index> index_hash;//索引的哈希图s

        uint64_t active_hint_id;
        uint64_t active_data_id;

        uint32_t active_data_pos;
        uint32_t acitve_hint_pos;
        
        const uint32_t max_file= 4096; 

        void write_hint_file(int64_t id,const bitcask_index& b_d);
        uint64_t write_data_file(int64_t id,const bitcask_data& b_d);

        void init_directory(const std::string &directory);
        int file_is_exist(const std::string & s);
        int new_folder(const std::string & s);

        void file_ostream();//将活跃的id的文件打开，用于之后的写入

        void map_create();//建立索引表

        int find_max_id(const std::string & directory , const std::string & file);
        //找到活跃的id（即文件名字中最大的id）

        bool read_file();


};

class error{
    public:

    error(short i,std::string msg);
    int getType(){
        return i;
    }
    std::string toString();
    
    private:
    short i;//1:not found;2:
    std::string msg;

};

std::string int_to_string(int64_t i);

}

#endif