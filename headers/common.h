#pragma once
#include <iostream>
#include <optional>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>

bool test_symbol(const std::string& s);
bool test_type(const std::string& s);
bool test_code(const std::string& s);

std::string load_file(const std::string& filename);
std::string parse_resource(const nlohmann::json& ref, const std::string& base);

struct packet_t{
    enum packet_t_naked{
        NONE, TUPLE, TYPE
    }data;

    packet_t(){data=NONE;}
    packet_t(packet_t_naked d):data(d){}

    packet_t& operator=(const std::string& v){
        if(v=="false")data=NONE;
        else if(v=="tuple")data=TUPLE;
        else if(v=="type")data=TYPE;
        else{
            std::cerr<<"Error, invalid packet type.\n";
            exit(1);
        }
        return *this;
    }

    operator packet_t_naked(){
        return data;
    }
};
