#include "common.h"

int main(){
    nlohmann::json specs;
    std::cin>>specs;

    std::stringstream _headers,_fields,_fns,_load,_unload,_tail;
    std::string CLASS_NAME,CONSTRUCTOR,DESTRUCTOR;
    std::vector<std::pair<std::string,bool>> SYMBOLS;
    std::string base;


    {
        auto t=specs.find("$base");
        if(t==specs.end()){}
        else if(!t->is_string()){
            std::cerr<<"Error, $base should be a string.\n";
            exit(1);
        }
        else{
            base=std::string(*t);
        }
    }

    {
        auto name=specs.find("$interface-name");
        if(name==specs.end()){
            std::cerr<<"Error, no name specified. Please add an $interface-name field.\n";
            exit(1);
        }
        else if(!name->is_string()){
            std::cerr<<"Error, $interface-name should be a string.\n";
            exit(1);
        }
        CLASS_NAME=std::string(*name);
    }

    {
        auto lang=specs.find("$lang");
        auto rev=specs.find("$rev");
        if(lang==specs.end()){
            std::cerr<<"Error, no language specified. Please add a $lang field.\n";
            exit(1);
        }
        else if(!lang->is_string()){
            std::cerr<<"Error, $lang should be a string.\n";
            exit(1);
        }
        else if(*lang!="c++"){
            std::cerr<<"Error, at the moment $lang can only be set to \"c++\".\n";
            exit(1);
        }

        if(rev==specs.end()){specs["$rev"]="20";}
    }

    //C++ Parser!

    //ctor
    {
        auto t=specs.find("constructor");
        if(t!=specs.end()){
            if(!t->is_string()){
                std::cerr<<"Error, [constructor] must be a string.\n";
                exit(1);
            }
            else{
                test_symbol(*t);
                CONSTRUCTOR=std::string(*t);
            }
        }
    }

    //dtor
    {
        auto t=specs.find("destructor");
        if(t!=specs.end()){
            if(!t->is_string()){
                std::cerr<<"Error, [destructor] must be a string.\n";
                exit(1);
            }
            else{
                test_symbol(*t);
                DESTRUCTOR=std::string(*t);
            }
        }
    }


    //Headers
    {
        auto headers=specs.find("headers");
        if(headers!=specs.end()){
            if(!headers->is_array()){
                std::cerr<<"Error, $headers must be an array of strings.\n";
                exit(1);
            }
            for(auto& i:*headers){
                if(!i.is_string()){
                    std::cerr<<"Error, items in $headers should be strings.\n";
                    exit(1);
                }
                _headers<<"#include \""<<std::string(i)<<"\"\n";
            }
        }
    }

    //Symbols
    {
        auto symbols=specs.find("symbols");
        if(symbols==specs.end()){
            std::cerr<<"Warning, no symbols defined.\n";
        }
        else if(!symbols->is_object()){
            std::cerr<<"Error, symbols must be stored in an associative map.\n";
            exit(1);
        }
        else{
            for(auto z=symbols->begin();z!=symbols->end();z++){
                auto& i=z.key();
                auto& j=z.value();
                test_symbol(i);
                SYMBOLS.push_back({i,false});
                auto stype=j.find("stype");
                if(stype==j.end()){
                    std::cerr<<"Error, no static type specified for symbol ["<<i<<"].\n";
                    exit(1);
                }
                if(!stype->is_string()){
                    std::cerr<<"Error, the static type for symbol ["<<i<<"] must be a string.\n";
                    exit(1);
                }

                //Parse a function
                if(*stype=="function"){
                    std::string rets, args, named_args, def="nullptr";
                    packet_t rpacket, apacket;
                    bool use_stl=true;

                    //Evaluate bits
                    {
                        auto w=j.find("use_stl");
                        if(w!=j.end() && w->is_boolean()){use_stl=*w;}
                        else if(w!=j.end()){
                            std::cerr<<"Error, if defined [use_stl] for symbol ["<<i<<"] must be boolean. By default is set [true].\n";
                            exit(1);
                        }
                    }
                    {
                        auto w=j.find("rets-pack");
                        if(w!=j.end() && w->is_string()){rpacket=std::string(*w);}
                        else if(w!=j.end()){
                            std::cerr<<"Error, when defined [rets-packet] for symbol ["<<i<<"] must be string. By default it is set to [false].\n";
                            exit(1);
                        }
                        if(!use_stl){
                            std::cerr<<"Error, at the moment packing return values is not supported without the STL. Its support will be added in the future.\n";
                        }
                    }
                    {
                        auto w=j.find("args-pack");
                        if(w!=j.end() && w->is_string()){apacket=std::string(*w);}
                        else if(w!=j.end()){
                            std::cerr<<"Error, when defined [args-packet] for symbol ["<<i<<"] must be string. By default it is set to [false].\n";
                            exit(1);
                        }
                        if(!use_stl){
                            std::cerr<<"Error, at the moment packing return values is not supported without the STL. Its support will be added in the future.\n";
                        }
                    }

                    //Rets
                    {
                        auto w=j.find("rets");
                        if(w==j.end()){
                            rets="void";
                        }
                        else if(!w->is_array()){
                            std::cerr<<"Error, when defined [rets] for symbol ["<<i<<"] must be an array. By default it will set the type to [void].\n";
                            exit(1);
                        }
                        else if(w->size()==0){rets="void";}
                        else if(!rpacket && w->size()>1){
                            std::cerr<<"Error, unless you package you return values for symbol ["<<i<<"], you cannot have more than one return type.\n";
                            exit(1);
                        }
                        else{
                            for(auto it=w->begin();it!=w->end();it++){
                                if(it->is_string()){
                                    test_type(*it);
                                    rets+=std::string(*it);
                                    if(it+1!=w->end())rets+=", ";
                                }
                                else if(it->is_object()){
                                    bool optional=false;
                                    std::string tname,tval,tdef;

                                    {
                                        auto q=it->find("name");
                                        if(q==it->end()){}/*Optional here*/
                                        else if(!q->is_string()){
                                            std::cerr<<"Error, if specified the name for a return item in ["<<i<<"] must be a string.\n";
                                            exit(1);
                                        }
                                        else{test_symbol(*q);tname=std::string(*q);}
                                    }

                                    {
                                        auto q=it->find("default");
                                        if(q==it->end()){}/*Optional here*/
                                        else if(!q->is_string()){
                                            std::cerr<<"Error, if specified the default value for a return item in ["<<i<<"] must be a string.\n";
                                            exit(1);
                                        }
                                        else{test_code(*q);tdef=std::string(*q);}
                                    }

                                    {
                                        auto q=it->find("type");
                                        if(q==it->end()){tval="int";}/*Optional here*/
                                        else if(!q->is_string()){
                                            std::cerr<<"Error, if specified the type for a return item in ["<<i<<"] must be a string.\n";
                                            exit(1);
                                        }
                                        else{test_type(*q);tval=std::string(*q);}
                                    }

                                    {
                                        auto q=it->find("optional");
                                        if(q==it->end()){}/*Optional here*/
                                        else if(!q->is_boolean()){
                                            std::cerr<<"Error, if specified the optional flag for a return item in ["<<i<<"] must be a boolean.\n";
                                            exit(1);
                                        }
                                        else{
                                            if(!use_stl){
                                                std::cerr<<"Error, at the moment optional types are not supported without STL. It will be implemented later.\n";
                                                exit(1);
                                            }
                                            optional=*q;
                                        }
                                    }

                                    if(optional)tval="std::optional<"+tval+">";
                                    rets+=tval;
                                    if(it+1!=w->end())rets+=", ";
                                }
                                else{
                                    std::cerr<<"Error, return types for symbol ["<<i<<"] can be strings or objects.\n";
                                    exit(1);
                                }
                            }
                            if(rpacket==packet_t::TUPLE && w->size()>1){
                                rets="std::tuple<"+rets+">";
                            }
                            else{
                                std::cerr<<"Error, composition method for rets in symbol ["<<i<<"] is not supported yet.\n";
                                exit(1);
                            }
                        }
                    }

                    //Args
                    {
                        auto w=j.find("args");
                        if(w==j.end()){
                            args="";
                            named_args="";
                        }
                        else if(!w->is_array()){
                            std::cerr<<"Error, when defined [args] for symbol ["<<i<<"] must be an array. By default it will set the type to [void].\n";
                            exit(1);
                        }
                        else{
                            uint index=0;
                            for(auto it=w->begin();it!=w->end();it++,index++){
                                if(it->is_string()){
                                    test_type(*it);
                                    args+=std::string(*it);
                                    named_args+=std::string(*it)+" _arg_"+std::to_string(index);
                                    if(it+1!=w->end()){args+=", ";named_args+=", ";}
                                }
                                else if(it->is_object()){
                                    bool optional=false;
                                    std::string tname,tval,tdef;

                                    {
                                        auto q=it->find("name");
                                        if(q==it->end()){tname="_arg_"+std::to_string(index);}/*Optional here*/
                                        else if(!q->is_string()){
                                            std::cerr<<"Error, if specified the name for a parameter item in ["<<i<<"] must be a string.\n";
                                            exit(1);
                                        }
                                        else{test_symbol(*q);tname=std::string(*q);}
                                    }

                                    {
                                        auto q=it->find("default");
                                        if(q==it->end()){}/*Optional here*/
                                        else if(!q->is_string()){
                                            std::cerr<<"Error, if specified the default value for a parameter item in ["<<i<<"] must be a string.\n";
                                            exit(1);
                                        }
                                        else{test_code(*q);tdef=std::string(*q);}
                                    }

                                    {
                                        auto q=it->find("type");
                                        if(q==it->end()){tval="int";}/*Optional here*/
                                        else if(!q->is_string()){
                                            std::cerr<<"Error, if specified the type for a parameter item in ["<<i<<"] must be a string.\n";
                                            exit(1);
                                        }
                                        else{test_type(*q);tval=std::string(*q);}
                                    }

                                    {
                                        auto q=it->find("optional");
                                        if(q==it->end()){}/*Optional here*/
                                        else if(!q->is_boolean()){
                                            std::cerr<<"Error, if specified the optional flag for a parameter item in ["<<i<<"] must be a boolean.\n";
                                            exit(1);
                                        }
                                        else{
                                            if(!use_stl){
                                                std::cerr<<"Error, at the moment optional types are not supported without STL. It will be implemented later.\n";
                                                exit(1);
                                            }
                                            optional=*q;
                                        }
                                    }

                                    if(optional)tval="std::optional<"+tval+">";
                                    args+=tval;
                                    named_args+=tval+" "+tname;
                                    if(it+1!=w->end()){args+=", ";named_args+=", ";}
                                }
                                else{
                                    std::cerr<<"Error, parameter types for symbol ["<<i<<"] can be strings or an objects.\n";
                                    exit(1);
                                }
                            }
                            if(apacket==packet_t::TUPLE && w->size()>1){
                                args="std::tuple<"+args+">";
                                named_args=args+"_private__args";    //No more name information here :(
                            }
                            else{
                                std::cerr<<"Error, composition method for args in symbol ["<<i<<"] is not supported yet.\n";
                                exit(1);
                            }
                        }
                    }

                    //Default function
                    {
                        auto w=j.find("default");
                        if(w==j.end()){def="nullptr";}
                        else{
                            auto tmp=parse_resource(*w,base);
                            test_code(tmp);
                            SYMBOLS.back().second=true;
                            def="+[]("+named_args+")->"+rets+"{"+tmp+"}"; //TODO integrate code here if I want it!
                        }
                    }

                    //Generation!
                    _fields<<"\t\t"<<rets<<" (*_private__"<<i<<")("<<args<<")="<<def<<";\n";
                    if(i!=CONSTRUCTOR && i!=DESTRUCTOR)_fns<<"\t\tconst decltype(_private__"<<i<<")& "<<i<<"=_private__"<<i<<";\n";
                }

                //Parse a variable
                else if(*stype=="variable"){
                    std::string tp,def;
                    {
                        auto w=j.find("type");
                        if(w==j.end())tp="int";  //Automatic selection of int as a type for variables.
                        else if(!w->is_string()){
                            std::cerr<<"Error, types for the variable ["<<i<<"] must be specified as a string.\n";
                            exit(1);
                        }
                        else{
                            test_type(*w);
                            tp=std::string(*w);
                        }
                    }
                    {
                        auto w=j.find("default");
                        if(w==j.end())def="nullptr";  //Automatic selection of int as a type for variables.
                        else{
                            auto tmp=parse_resource(*w,base);
                            test_code(tmp);
                            SYMBOLS.back().second=true;
                            def="_static__"+i;
                            _fields<<"\t\tinline static "<<tp<<" "<<def<<"="<<std::string(*w)<<";\n";
                        }
                    }
                    _fields<<"\t\t"<<tp<<"* _private__"<<i<<"=&"<<def<<";\n";
                    _fns<<"\t\tinline "<<tp<<"& "<<i<<"(){return *_private__"<<i<<";}\n";
                }
                else{
                    std::cerr<<"Error, static type ["<<std::string(*stype)<<"] for symbol ["<<i<<"] is not supported.\n";
                    exit(1); 
                }
            }
        }
    }

    {
        auto t=specs.find("tail");
        if(t==specs.end()){}
        else{
            _tail<<parse_resource(*t,base);
        }
    }

    _headers<<"#include <optional>\n#include <string>\n#include <tuple>\n#include <dlfcn.h>\n\n";
    _fns<<"\t\ttemplate<typename ...Args>\n\t\t"<<CLASS_NAME<<"(std::string filename,Args... args){\n";
    _fns<<"\t\t\thandle=dlopen(filename.c_str(),RTLD_LAZY);\n"
        <<"\t\t\tif(!handle){auto* err=dlerror();std::cerr<<\"Error, unable to open [\"<<filename<<\"]. Exception thrown.\\n\";throw StringException(\"DLOPEN_failure\");}\n\t\t\tdlerror();\n\n";
    //Cycle all the symbols to be loaded!
    for(auto& i:SYMBOLS){
        _fns<<"\t\t\t{auto old=_private__"<<i.first<<";_private__"<<i.first<<"=(decltype(_private__"<<i.first<<"))dlsym(handle,\""<<i.first<<"\");auto* err=dlerror();if(!"<<i.second<<" && err){std::cerr<<\"Unable to load the symbol ["<<i.first<<"]. Exception thrown.\\n\";throw StringException(\"SymbolLoad_failure\");}if(err){_private__"<<i.first<<"=old;}}\n";
    }

    if(CONSTRUCTOR!="")_fns<<"\n\t\t\t_private__"<<CONSTRUCTOR<<"(args...);\n";
    _fns<<"\t\t}\n\n";
    _fns<<"\t\t~"<<CLASS_NAME<<"(){";
    if(DESTRUCTOR!="")_fns<<"_private__"<<DESTRUCTOR<<"();";    
    _fns<<"}\n";

    _fields<<"\t\tvoid* handle=nullptr;\n";


    std::cout<<_headers.str();

    std::cout<<"#include <string>\n\
    struct StringException : public std::exception{\n\
    std::string s;\n\
    StringException(std::string ss) : s(ss) {}\n\
    ~StringException() throw () {} // Updated\n\
    const char* what() const throw() { return s.c_str(); }\n\
    };\n\n";

    std::cout<<"struct "<<CLASS_NAME<<"{\n\tprivate:\n";
    std::cout<<_fields.str();
    std::cout<<"\tpublic:\n";
    std::cout<<_fns.str();
    std::cout<<"};\n";
    std::cout<<_tail.str()<<"\n";
    return 0;
}