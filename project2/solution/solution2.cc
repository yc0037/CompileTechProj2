#include <fstream>
#include <json.hpp>
#include <dirent.h>
#include "parser.tab.h"
#include "syntaxtree.hpp"

using json = nlohmann::json;

extern Function * mainFunc;
extern int id;

int main()
{
    string casespath = "./cases";
    string kernelpath = "./kernels";
    DIR* dir = opendir(casespath.data());
    struct dirent* dirp;
    string filename;
    while ((dirp = readdir(dir)) != NULL)
    {
        filename = dirp->d_name;
        if (dirp->d_type != DT_REG) continue;
        ifstream ifile(casespath + '/' + filename);
        if (! ifile.is_open()) continue;
        
        json ir;
        ifile >> ir;
        ifile.close();
        std::string sname;
        ir["name"].get_to(sname);
        std::vector<std::string> ins;
        ir["ins"].get_to(ins);
        std::vector<std::string> outs;
        ir["outs"].get_to(outs);
        std::string sdata_type;
        ir["data_type"].get_to(sdata_type);
        Type data_type = Type::float_scalar(32);
        if (sdata_type == "int") data_type = Type::int_scalar(32);
        std::string skernel;
        ir["kernel"].get_to(skernel);

        FILE* tmp = fopen("tmp", "w");
        fwrite(skernel.data(), sizeof(char), skernel.size(), tmp);
        fclose(tmp);

        extern FILE* yyin;
        yyin = fopen("tmp", "r");

        mainFunc = new Function(id++, sname, data_type, ins, outs);

        yyparse();
        
        Group kernel = mainFunc -> makeGroup();

        IRVisitor visitor;
        kernel.visit_group(&visitor);

        IRMutator mutator;
        kernel = mutator.mutate(kernel);

        IRPrinter printer;
        string code = printer.print(kernel);

        ofstream ofile(kernelpath + '/' + sname + ".cc");
        ofile << code;
        ofile.close();
        delete mainFunc;
    }
    closedir(dir);
    return 0;
}