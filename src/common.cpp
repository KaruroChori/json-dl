#include "common.h"

bool test_symbol(const std::string& s){return true;}
bool test_type(const std::string& s){return true;}
bool test_code(const std::string& s){return true;}

std::string load_file(const std::string& filename){
    std::ifstream file(filename);
    if(!file){
        std::cerr<<"Error file ["<<filename<<"] cannot be opened.\n";
        exit(1);
    }
    return std::string((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());
}

std::string parse_resource(const nlohmann::json& ref, const std::string& base){
    //By default it is inline.
    if(ref.is_string())return std::string(ref);
    else if(ref.is_object()){
        auto it1=ref.find("path");
        auto it2=ref.find("inline");
        if(it1!=ref.end() && it2!=ref.end()){
            std::cerr<<"Error, path and inline cannot be declared at the same time for a resource.\n";
            exit(1);
        }
        else if (it1!=ref.end()){
            if(!it1->is_string()){
                std::cerr<<"Error, the path field must be a string.\n";
                exit(1);
            }
            return load_file(base+std::string(*it1));
        }
        else if (it2!=ref.end()){
            if(!it2->is_string()){
                std::cerr<<"Error, the inline field must be a string.\n";
                exit(1);
            }
            return std::string(*it2);
        }
        else{
            std::cerr<<"Error, path xor inline must be declared for a resource.\n";
            exit(1);
        }
    }
    else{
        std::cerr<<"Error, a string or an object were expected as resource for code.\n";
        exit(1);
    }
    return "";
}