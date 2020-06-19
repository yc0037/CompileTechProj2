#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "IR.h"
#include "IRMutator.h"
#include "IRVisitor.h"
#include "IRPrinter.h"
#include "type.h"
using namespace std;
using namespace Boost::Internal;

enum Opt {ADD, SUB, MUL, FDIV, MOD, IDIV, BRACKET, NONE};

/*
struct Cond
{
    Node* expr;
    unsigned long range;
};
*/

// EDITED
class Tref;
typedef std::map<std::string, Tref*> dTrefMap;
extern dTrefMap dTrefs;
extern std::string curLeft;
extern std::string curGrad;
extern std::vector<std::string> sgradto;


class Node {
    public:
    unsigned long nid;
    string type;

    Node(int i, string t) : nid(i),type(t) { 
        //cout << "id:" << i << " type:" << t << " created" << endl;
    }
    virtual Expr makeExpr(bool flag, map<string, Expr> &IndexTable, Type& data_type) = 0;
    virtual ~Node() {};
};

class Integer : public Node {
    public:
    unsigned long value;
    Integer(int i, unsigned long v) : Node(i, "Integer"), value(v) { }

    Expr makeExpr(bool flag, map<string, Expr> &IndexTable, Type& data_type) {
        return IntImm::make(Type::int_scalar(32), value);
    }
};

class Float : public Node {
    public:
    string fvalue;
    Float(int i, string fv) :  Node(i, "Float"), fvalue(fv) {}

    Expr makeExpr(bool flag, map<string, Expr> &IndexTable, Type& data_type) {
        // EDITED
        return StringImm::make(data_type, "0.0");
    }
};

class Identifier : public Node {
    public:
    string name;

    Identifier(int i, string n) :  Node(i, "Identifier"), name(n) { }

    Expr makeExpr(bool flag, map<string, Expr> &IndexTable, Type& data_type) {
        return IndexTable[name];
    }
};

class Operator : public Node {
    public:
    Opt op;

    Operator(int i, Opt o) : Node(i, "Opt"), op(o) {}

    Expr makeExpr(bool flag, map<string, Expr> &IndexTable, Type& data_type) {
        return Var::make(data_type, "", {}, {});
    }
};

class IdExpr : public Node {
    public:
    Node * left;
    Node * right;
    Opt op;

    IdExpr(int i, Node *l, Opt o, Node *r) : Node(i,"IdExpr"), left(l), right(r), op(o){}

    ~IdExpr() {
        if (left != NULL) delete left;
        left = NULL;
        if (right != NULL) delete right;
        right = NULL;
    }
    /*
    struct Cond collectRange(unsigned long range) {
        if (right != NULL && right -> type == "Integer") {
            switch (op)
            {
            case ADD:
                range -=  ((Integer*)right) -> value;
                break;
            case SUB:
                range += ((Integer*)right) -> value;
                break;
            case MUL:
                range /= ((Integer*)right) -> value;
                break;
            case IDIV:
                range *= ((Integer*)right) -> value;
                break;
            default:
                struct Cond cond = {this, range};
                return cond;
            }
            if (left -> type == "IdExpr")
                return ((IdExpr *) left) -> collectRange(range);
            else  {
                struct Cond cond = {left, range};
                return cond;
            }
        }
        else if (right != NULL && right -> type == "IdExpr"){
            struct Cond cond = {this, range};
            return cond;
        } else if (right == NULL && left -> type == "Identifier"){
            struct Cond cond = {left, range};
            return cond;
        } else {
            return ((IdExpr *) left) -> collectRange(range);
        }
    }

    set<string> & collectIdx(set<string> &idxs) {
        if (right -> type == "IdExpr") {
            ((IdExpr*)right) -> collectIdx(idxs);
        }
        if (left -> type == "IdExpr") {
            ((IdExpr*)left) -> collectIdx(idxs);
        } else {
            idxs.insert(((Identifier*)left)->name);
        }
    }*/
    void checkRange(unsigned long range, map<string, unsigned long> & idxMap) {
        //cout << "checking Idexpr " << nid << endl;
        if (right != NULL && right -> type == "Integer") {
            switch (op)
            {
            case ADD:
                range -=  ((Integer*)right) -> value;
                break;
            case SUB:
                range += ((Integer*)right) -> value;
                break;
            case MUL:
                range /= ((Integer*)right) -> value;
                break;
            case IDIV:
                range *= ((Integer*)right) -> value;
                break;
            default:
                break;
            }
            ((IdExpr *) left) -> checkRange(range, idxMap);
        }
        else if (right != NULL && right -> type == "IdExpr"){
            ((IdExpr *) left) -> checkRange(range, idxMap);
            ((IdExpr *) right) -> checkRange(range, idxMap);
        } else if (right == NULL && left -> type == "Identifier"){
            string idx = ((Identifier *)left) -> name;
            map<string, unsigned long> :: iterator it;
            it = idxMap.find(idx);
            if (it != idxMap.end() && it -> second > range) {
                it -> second = range;
            } else {
                idxMap.insert(pair<string, unsigned long>(idx, range));
            }
        } else {
            return ((IdExpr *) left) -> checkRange(range, idxMap);
        }
    }

    Expr makeExpr (bool flag, map<string, Expr> &IndexTable, Type& data_type) {
        if (right != NULL) {
            BinaryOpType  op_type = BinaryOpType::Add;
            switch (op)
            {
            case ADD:
                break;
            case SUB:
                op_type = BinaryOpType::Sub;
                break;
            case MUL:
                op_type = BinaryOpType::Mul;
                break;
            case IDIV:
                op_type = BinaryOpType::Div;
                break;
            case MOD:
                op_type = BinaryOpType::Mod;
                break;
            default:
                break;
            }
            return Binary::make(Type::int_scalar(32), op_type, left->makeExpr(false, IndexTable, data_type), right->makeExpr(false, IndexTable, data_type), flag);
        } else if (op == BRACKET) {
            return left -> makeExpr(true, IndexTable, data_type);
        }   else {
            return left -> makeExpr(false,IndexTable, data_type);
        }
    }
};

class AList : public Node {
    public:
    vector<Node*> nodelist;

    AList(int i) : Node(i, "AList") {}

    void insertNode(Node* node) {
        nodelist.insert(nodelist.begin(), node);
    }

    Expr makeExpr(bool flag, map<string, Expr> &IndexTable, Type& data_type) {
        return Var::make(data_type, "", {}, {});
    }
};

class CList : public Node {
    public:
    vector<unsigned long> numlist;

    CList(int i) : Node(i, "CList") {}

    void insertNum(unsigned long u) {
        numlist.insert(numlist.begin(), u);
    }

    Expr makeExpr(bool flag, map<string, Expr> &IndexTable, Type& data_type) {
        return Var::make(data_type, "", {}, {});
    }
};

class Tref : public Node {
    public:
    string name;
    vector<unsigned long> ranges;
    vector<Node*> idExprs;
    //vector<Node*> simExprs;

    Tref(int i, string n): Node(i, "Tref"), name(n) {}
    
    ~Tref() {
        for (vector<Node*> :: iterator it = idExprs.begin(); it != idExprs.end(); it++) {
            if ((*it) != NULL) 
                delete (*it);
            *it = NULL;
        }
    }

    void insertSize(unsigned long s) {
        ranges.push_back(s);
    }

    void insertExpr(Node* n) {
        idExprs.push_back(n);
    }

    void checkRange(map<string, unsigned long> & idxMap, map<string, vector<unsigned long> > &TrefTable) {
        //cout << "checking Tref " << nid << endl;
        map<string, vector<unsigned long> > :: iterator it = TrefTable.find(name);
        if (it == TrefTable.end()) {
            TrefTable.insert(pair<string, vector<unsigned long> >(name, ranges));
        }
        unsigned long l = ranges.size();
        for (unsigned i = 0; i < l; i++) {
            ((IdExpr*) idExprs[i]) -> checkRange(ranges[i], idxMap);
        }

        // EDITED
        dTrefMap::iterator dit = dTrefs.find(name);
        if (dit == dTrefs.end()) {
            Tref* dCur = new Tref(-1, "d" + name);
            dCur->ranges = this->ranges;
            dCur->idExprs = this->idExprs;
            dTrefs.insert(pair<string, Tref*>(name, dCur));
        }
    }

    Expr makeExpr(bool flag, map<string, Expr> &IndexTable, Type& data_type) {
        // EDITED
        Tref* retTref;
        if (this->name == curLeft) {
            retTref = dTrefs.find(curGrad)->second;
        } else if (this->name == curGrad) {
            retTref = dTrefs.find(curLeft)->second;
        } else {
            retTref = this;
        }
        vector<Expr> exprs;
        unsigned long l = retTref->idExprs.size();
        for (unsigned i = 0; i < l; i++) {
            exprs.push_back(retTref->idExprs[i] -> makeExpr(false, IndexTable, data_type));
        }
        return Var::make(data_type, retTref->name, exprs, retTref->ranges);
        // vector<Expr> exprs;
        // unsigned long l = idExprs.size();
        // for (unsigned i = 0; i < l; i++) {
        //     exprs.push_back(idExprs[i] -> makeExpr(false, IndexTable, data_type));
        // }
        // return Var::make(data_type, name, exprs, ranges);
    }
};

class Sref : public Node {
    public:
    string name;

    Sref(int i, string n) : Node(i, "Sref"), name(n) {}

    Expr makeExpr(bool flag, map<string, Expr> &IndexTable,  Type& data_type) {
        return Var::make(data_type, name, {}, {1});
    } 
};

class RHS : public Node {
    public:
    Node * left;
    Node * right;
    Opt op;

    RHS(int i, Node *l, Opt o, Node *r) : Node(i,"RHS"), left(l), right(r), op(o) {}

    ~RHS() {
        if (left != NULL) delete left;
        left = NULL;
        if (right != NULL) delete right;
        right = NULL;
    }

    void checkRange(map<string, unsigned long> & idxMap, map<string, vector<unsigned long> > &TrefTable) {
        //cout << "checking RHS " << nid << endl;
        if (right != NULL && right -> type == "RHS") {
            ((RHS*)right) -> checkRange(idxMap, TrefTable);
        }
        if (left -> type == "RHS") {
            ((RHS*)left) -> checkRange(idxMap, TrefTable);
        } else if (left -> type == "Tref") {
            ((Tref*)left) -> checkRange(idxMap, TrefTable);
        }
    }

    Expr makeExpr (bool flag, map<string, Expr> &IndexTable, Type& data_type) {
        if (right != NULL) {
            BinaryOpType op_type = BinaryOpType::Add;
            switch (op)
            {
            case ADD:
                // if (left -> type == "Integer") {
                //     ((Integer*) left) -> value = 0;
                // } else if (right -> type == "Integer") {
                //     ((Integer*) right) -> value = 0;
                // } else if (left -> type == "Float") {
                //     ((Float*) left) -> fvalue = "0.0";
                // } else if (right -> type == "Float") {
                //     ((Float*) right) -> fvalue = "0.0";
                // }
                break;
            case SUB:
                // if (left -> type == "Integer") {
                //     ((Integer*) left) -> value = 0;
                // } else if (right -> type == "Integer") {
                //     ((Integer*) right) -> value = 0;
                // } else if (left -> type == "Float") {
                //     ((Float*) left) -> fvalue = "0.0";
                // } else if (right -> type == "Float") {
                //     ((Float*) right) -> fvalue = "0.0";
                // }
                op_type = BinaryOpType::Sub;
                break;
            case MUL:
                op_type = BinaryOpType::Mul;
                break;
            case IDIV:
            case FDIV:
                op_type = BinaryOpType::Div;
                break;
            case MOD:
                op_type = BinaryOpType::Mod;
                break;
            default:
                break;
            }
            return Binary::make(data_type, op_type, left->makeExpr(false, IndexTable, data_type), right->makeExpr(false, IndexTable, data_type), flag);
        } else if (op == BRACKET) {
            return left -> makeExpr(true, IndexTable, data_type);
        }   else {
            return left -> makeExpr(false, IndexTable, data_type);
        }
    }
};

class MoveStmt : public Node {
    public:
    Node * left;
    Node * right;
    bool ifConv;
    map<string, unsigned long> leftIdxMap;
    map<string, unsigned long> rightIdxMap;
    map<string, Expr> IndexTable;

    MoveStmt(int i, Node *l, Node *r) : Node(i,"MStmt"), left(l), right(r), ifConv(false) { }

     ~MoveStmt() {
        if (left != NULL) delete left;
        left = NULL;
        if (right != NULL) delete right;
        right = NULL;
    }

    void checkRange(map<string, vector<unsigned long> > &TrefTable) {
        //cout << "checking MoveStmt " << nid << endl;
        ((Tref*)left) -> checkRange(leftIdxMap, TrefTable);
        ((RHS*)right) -> checkRange(rightIdxMap, TrefTable);
        for (map<string, unsigned long>::iterator it = leftIdxMap.begin(); it != leftIdxMap.end(); it++) {
            Expr tmp_dom = Dom::make(Type::int_scalar(32), 0, (int)(it -> second), it -> first);
            Expr tmp = Index::make(Type::int_scalar(32), it -> first, tmp_dom, IndexType::Spatial);
            IndexTable.insert(pair<string, Expr>(it->first, tmp));
        }
        for (map<string, unsigned long>::iterator it = rightIdxMap.begin(); it != rightIdxMap.end(); it++) {
            if (leftIdxMap.find(it->first) == leftIdxMap.end()) {
                ifConv = true;
                Expr tmp_dom = Dom::make(Type::int_scalar(32), 0, (int)(it -> second), it -> first);
                Expr tmp = Index::make(Type::int_scalar(32), it -> first, tmp_dom, IndexType::Spatial);
                IndexTable.insert(pair<string, Expr>(it->first, tmp));
            }
        }
    }

    Stmt makeStmt(Type& data_type) {
        // EDITED
        vector<Stmt> movestmts;
        for (std::string grad_to : sgradto) {
            curGrad = grad_to;
            Stmt movestmt = Move::make(left->makeExpr(false, IndexTable, data_type), right->makeExpr(false, IndexTable, data_type), ifConv?MoveType::MemToShared:MoveType::MemToMem);
            movestmts.push_back(movestmt);
        }

        // Stmt movestmt = Move::make(left->makeExpr(false, IndexTable, data_type), right->makeExpr(false, IndexTable, data_type), MoveType::MemToMem);
        vector<Expr> indexs;
        for (map<string, Expr> :: iterator it = IndexTable.begin(); it != IndexTable.end(); it++) {
            indexs.push_back(it->second);
        }
        return LoopNest::make(indexs, movestmts);
    }

    Expr makeExpr(bool flag, map<string, Expr> &IndexTable, Type& data_type) {
        return Var::make(data_type, "", {}, {});
    }
};

class Function : public Node {
    public:
    string name;
    Type data_type;
    vector<string> ins;
    vector<string> outs;
    vector<Node *> stmtNodes;
    map<string, vector<unsigned long> > TrefTable;

    Function(int i, string n, Type t, vector<string> &in, vector<string> &out) : Node(i, "Function"), name(n), data_type(t), ins(in), outs(out) {}

    ~Function() {
        for (vector<Node*> :: iterator it = stmtNodes.begin(); it != stmtNodes.end(); it++) {
            if ((*it) != NULL) 
                delete (*it);
            *it = NULL;
        }
    }

    void insertStmt(Node* stmt) {
        stmtNodes.insert(stmtNodes.begin(), stmt);
    }

    Group makeGroup() {
        vector<Stmt> stmts;
        vector<Expr> input;
        vector<Expr> output;

        // EDITED
        vector<std::string> inputSet;

        for (unsigned i = 0; i < stmtNodes.size(); i++) {
            ((MoveStmt *) stmtNodes[i]) -> checkRange(TrefTable);
            //cout << "checked" << endl;
            stmts.push_back(((MoveStmt *) stmtNodes[i]) -> makeStmt(data_type));
        }
        for (unsigned j = 0; j < outs.size(); j++) {
            // EDITED
            string tmpName = outs[j];
            if(tmpName == curLeft && find(inputSet.begin(), inputSet.end(), tmpName) == inputSet.end()) {
                inputSet.push_back("d" + tmpName);
            }

            // string tmpName = outs[j];
            // Expr tref = Var::make(data_type, tmpName, {}, TrefTable[tmpName]);
            // output.push_back(tref);
        }
        for (unsigned k = 0; k < ins.size(); k++) {
            // EDITED
            string tmpName = ins[k];
            if (find(outs.begin(), outs.end(), tmpName) == outs.end()) {
                if(find(sgradto.begin(), sgradto.end(), tmpName) != sgradto.end()){
                    Expr tref = Var::make(data_type, "d" + tmpName, {}, TrefTable[tmpName]);
                    output.push_back(tref);
                    for (std::string iin : ins) {
                        if(iin == tmpName || find(inputSet.begin(), inputSet.end(), iin) != inputSet.end()) continue;
                        inputSet.push_back(iin);
                    }
                }
            }

            // string tmpName = ins[k];
            // if (find(outs.begin(), outs.end(), tmpName) == outs.end()) {
            //     Expr tref = Var::make(data_type, tmpName, {}, TrefTable[tmpName]);
            //     input.push_back(tref);
            // }
        }
        sort(inputSet.begin(), inputSet.end());
        for (int i = 0; i < inputSet.size(); ++i) {
            std::string TrefName = inputSet[i];
            if(TrefName[0] == 'd')
                TrefName = TrefName.substr(1);
            Expr tref = Var::make(data_type, inputSet[i], {}, TrefTable[TrefName]);
            input.push_back(tref);
        }
        return Kernel::make(name, input, output, stmts, KernelType::CPU);
    }

    Expr makeExpr(bool flag, map<string, Expr> &IndexTable, Type& data_type) {
        return Var::make(data_type, "", {}, {});
    }
};
/*
class Unit : public Node {
    public:
    vector<Node*> nodeList;
    vector<Opt> opList;
    map<string, unsigned long> needIdx;
    map<string, unsigned long> extraCond;

    Unit(int i) : Node(i, "Unit") {}
};*/
