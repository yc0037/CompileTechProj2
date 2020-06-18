#include <fstream>
#include <stdlib.h>
#include <json.hpp>
#include <memory>
#include "parser.tab.h"
#include "syntaxtree.hpp"

using json = nlohmann::json;

extern Function * mainFunc;
extern int id;

int main()
{
    Type data_type = Type::float_scalar(32);
    ifstream ifile("../../project1/cases/case1.json");
    if (! ifile.is_open())
    { cout << "Error opening file"; exit (1); }
    
    json ir;
    ifile >> ir;
    std::string sname;
    ir["name"].get_to(sname);
    std::vector<std::string> ins;
    ir["ins"].get_to(ins);
    std::vector<std::string> outs;
    ir["outs"].get_to(outs);
    std::string sdata_type;
    ir["data_type"].get_to(sdata_type);
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

    cout << code;
    delete mainFunc;
    return 0;
}