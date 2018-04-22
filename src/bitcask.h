//小试牛刀，试试bitcask的源码编写

#ifdef _BITCASK_H
#define _BITCASK_H

#include<fstream>
#include<unordered_map>
#include<cstring>
#include<vector>
#include<time.h>
#include<unistd.h>
namespace bitcask{



struct bitcask_data{
    time_t time;//增加数据的时间
    int64_t key_len;//key的长度
    std::string key;//数据的索引
    int64_t value_len;//数据段的长度
    std::string value;//数据
};//存储数据的结构体

struct bitcask_index{
    time_t time;//增加数据的时间
    bool flag;//是否已被删除
    int64_t key_len;//key的长度
    std::string key;//数据的索引
    int64_t file_id;//数据所在位置文件的id
    int64_t file_pos;//数据所在文件的位置
};//存储数据位置的结构体

class Bitcask{
    public:
        Bitcask();
        ~Bitcask();

        bool close();

        init();

        bool _insert();//增操作
        bool _update();//改操作
        bool _delete();//删操作
        bool _select();//查操作
        bool _merge();//整合数据

    private:
        Bitcask(Bitcask &b);

        std::ostream active_data_file;
        std::ostream active_hint_file;
        
        unordered_map<std::string,bitcask_index> index;

        int64_t active_hint_id;
        int64_t active_data_id;

        bool write_file();
        bool read_file();


};





}
#endif