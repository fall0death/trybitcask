//
// Created by liiiyu on 2018/4/23.
//

#include "bitcask.h"

int main() {
    bitcask::Bitcask b;
    b.init();
    while(1){
        try{
            int i;
            std::cout<<"1:insert,2:update,3:delete,4:select"<<std::endl;
            std::cin>>i;
            std::string key;
            std::string value;
            time_t _time;
            switch(i){
                case 1:
                    std::cout<<"write down your key"<<std::endl;
                    std::cin>>key;
                    std::cout<<"write down the value"<<std::endl;
                    std::cin>>value;
                    b._insert(key,value);
                break;
                case 2:
                    std::cout<<"write down the key"<<std::endl;
                    std::cin>>key;
                    std::cout<<"write down the value"<<std::endl;
                    std::cin>>value;
                    b._update(key,value);
                break;
                case 3:
                    std::cout<<"write down the key"<<std::endl;
                    std::cin>>key;
                    b._delete(key);
                break;
                case 4:
                    std::cout<<"write down the key"<<std::endl;
                    std::cin>>key;
                    b._select(key,_time,value);
                    std::cout<<"the value is: "+value<<std::endl;
                    std::cout<<"the time is: ";
                    std::cout<<_time<<std::endl;
                break;
                default:
                    std::cout<<"bye"<<std::endl;
                    return 0;
                break;
            }
        }catch(bitcask::error e){
            std::cout<<e.toString()<<std::endl;
        }
    }
}