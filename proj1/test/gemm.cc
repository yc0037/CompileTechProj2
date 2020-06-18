#include <string>
#include <iostream>

#include "IR.h"
#include "IRMutator.h"
#include "IRVisitor.h"
#include "IRPrinter.h"
#include "type.h"

using namespace Boost::Internal;

int main() {
    const int M = 1024;
    const int N = 512;
    const int K = 256;
    Type index_type = Type::int_scalar(32);
    Type data_type = Type::float_scalar(32);
    Type bool_type = Type::uint_scalar(1);

    // index i
    Expr dom_i = Dom::make(index_type, 0, M, "i");
    Expr i = Index::make(index_type, "i", dom_i, IndexType::Spatial);

    // index j
    Expr dom_j = Dom::make(index_type, 0, N, "j");
    Expr j = Index::make(index_type, "j", dom_j, IndexType::Spatial);

    // index k
    Expr dom_k = Dom::make(index_type, 0, K, "k");
    Expr k = Index::make(index_type, "k", dom_k, IndexType::Reduce);

    // A
    Expr expr_A = Var::make(data_type, "A", {i, k}, {M, K});

    // B
    Expr expr_B = Var::make(data_type, "B", {k, j}, {K, N});

    // C
    Expr expr_C = Var::make(data_type, "C", {i, j}, {M, N});

    // main stmt
    Stmt main_stmt = Move::make(
        expr_C,
        Binary::make(data_type, BinaryOpType::Add, expr_C,
            Binary::make(data_type, BinaryOpType::Mul, expr_A, 
                Unary::make(data_type, UnaryOpType::Neg, expr_B))),
        MoveType::MemToMem
    );

    // loop nest
    Stmt loop_nest = LoopNest::make({i, j, k}, {main_stmt});

    // Select test
    Expr expr_m = Var::make(index_type, "m", {}, {1});
    Expr const_m = IntImm::make(index_type, 5);
    Expr const_tsel = IntImm::make(index_type, 0);
    Expr const_fsel = IntImm::make(index_type, 1);
    Expr expr_cond = Compare::make(bool_type, CompareOpType::EQ, expr_m, const_m);
    Expr expr_sel = Select::make(index_type, expr_cond, const_tsel, const_fsel);
    Expr expr_s = Var::make(index_type, "s", {}, {1});
    Stmt sel = Move::make(expr_s, expr_sel, MoveType::LocalToLocal);

    // bracket test
    Expr expr_q = Var::make(index_type, "q", {}, {1});
    Expr expr_r = Var::make(index_type, "r", {}, {1});
    Expr expr_u = Var::make(index_type, "u", {}, {1});
    Stmt brkt = Move::make(
        expr_q, 
        Binary::make(index_type, BinaryOpType::Mul, expr_q,
            Binary::make(index_type, BinaryOpType::Add, expr_r, expr_u, true),
        false), 
        MoveType::LocalToLocal
    );

    // kernel
    Group kernel = Kernel::make("simple_gemm", {expr_A, expr_B}, {expr_C}, {loop_nest, sel, brkt}, KernelType::CPU);

    // visitor
    IRVisitor visitor;
    kernel.visit_group(&visitor);

    // mutator
    IRMutator mutator;
    kernel = mutator.mutate(kernel);

    // printer
    IRPrinter printer;
    std::string code = printer.print(kernel);

    std::cout << code;

    std::cout << "Success!\n";
    return 0;
}