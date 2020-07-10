#include "common.h"

int main(){
    nlohmann::json specs,source;
    std::cin>>specs;

    std::stringstream _headers, _vars, _proto, _fns, _tail;
    std::set<std::string> target_symbols;
    std::map<std::string,std::string> impl_table;
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

    {
        auto t=specs.find("source");
        if(t==specs.end()){
            std::cerr<<"Error, no source file specified. Please add a $source field.\n";
            exit(1);
        }
        else if(!t->is_string()){
            std::cerr<<"Error, $source should be a string.\n";
            exit(1);
        }
        std::ifstream source_f((base+std::string(*t)).c_str());
        if(!source_f){
            std::cerr<<"Error, unable to open the source file ["<<std::string(*t)<<"].\n";
            exit(1);
        }
        source_f>>source;
    }

    {
        auto t=specs.find("tail");
        if(t==specs.end()){}
        else{_tail<<parse_resource(*t,base);}
    }

    //Load all the implementations provided
    {
        auto t=specs.find("impl");
        if(t==specs.end()){
            std::cerr<<"Error, there must be an implementation block defined!\n";
            exit(1);
        }
        else if(!t->is_object()){
            std::cerr<<"Error, the implementation block must be an object.\n";
            exit(1);
        }
        else{
            for(auto it=t->begin();it!=t->end();it++){
                auto code=parse_resource(it.value(),base);
                test_code(code);
                impl_table.insert({it.key(),code});
            }
        }
    }

    //Read the original specifications.
    {
        std::vector<std::pair<std::string,bool>> SYMBOLS;

        {
            auto symbols=source.find("symbols");
            if(symbols==source.end()){
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
                        std::stringstream argspack;
                        std::string rets, args, named_args, def="";
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
                                        if(apacket)argspack<<std::string(*it)<<"& _arg_"<<index<<"=std::get<"<<index<<">(_private__args);";
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
                                        if(apacket)argspack<<tval<<"& "<<tname<<"=std::get<"<<index<<">(_private__args);";
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
                            if(w==j.end()){def="";}
                            else{
                                auto tmp=parse_resource(*w,base);
                                test_code(tmp);
                                SYMBOLS.back().second=true;
                                def=tmp;
                            }
                        }
                        //TODO: In case of a pack parameter generate the references to the differet elements!!!!

                        //Generation!
                        _proto<<"\t"<<rets<<" "<<i<<"("<<named_args<<");\n";

                        auto fv=impl_table.find(i);
                        if(fv==impl_table.end()){
                            if(def!=""){_fns<<"\t"<<rets<<" "<<i<<"("<<named_args<<"){\n"<<argspack.str()<<"\n"<<def<<"\n\t}\n";}
                            else {std::cerr<<"Error, symbol ["<<i<<"] has no value assigned. Functions must provide a proper implementation.\n";exit(1);}
                        }
                        else{
                            _fns<<"\t"<<rets<<" "<<i<<"("<<named_args<<"){\n"<<argspack.str()<<"\n"<<def<<"\n\t}\n";
                        }
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
                            if(w==j.end())def="";  //Automatic selection of int as a type for variables.
                            else{
                                auto tmp=parse_resource(*w,base);
                                test_code(tmp);
                                SYMBOLS.back().second=true;
                                def=tmp;
                            }
                        }
                        
                        auto fv=impl_table.find(i);
                        if(fv==impl_table.end()){
                            if(def!=""){_vars<<"\t"<<tp<<" "<<i<<"="<<def<<";\n";}
                            else {_vars<<"\t"<<tp<<" "<<i<<";\n";std::cerr<<"Warning, symbol ["<<i<<"] has no value assigned.\n";}
                        }
                        else{
                            _vars<<"\t"<<tp<<" "<<i<<"="<<fv->second<<";\n";
                        }
                    }
                    else{
                        std::cerr<<"Error, static type ["<<std::string(*stype)<<"] for symbol ["<<i<<"] is not supported.\n";
                        exit(1); 
                    }
                }
            }
        }

    }

    _headers<<"#include <optional>\n#include <string>\n#include <tuple>\n#include <dlfcn.h>\n\n";


    std::cout<<_headers.str();
    std::cout<<"extern \"C\"{\n";
    std::cout<<"\t//Variables\n"<<_vars.str()<<"\n\n";
    std::cout<<"\t//Prototypes\n"<<_proto.str()<<"\n\n";
    std::cout<<"\t//Implementation\n"<<_fns.str()<<"\n\n";
    std::cout<<"}\n\n";
    std::cout<<"//Tail\n"<<_tail.str()<<"\n";
    return 0;
}