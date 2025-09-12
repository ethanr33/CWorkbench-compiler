
#pragma once

#include "ASTNodes.h"

class AST;

class NodeVisitor {
    public:
        virtual void visit(ASTRootNode&) = 0;
        virtual void visit(ASTFunctionNode&) = 0;
        virtual void visit(ASTReturnNode&) = 0;
        virtual void visit(ASTTypeNode&) = 0;
        virtual void visit(ASTBinaryOpNode&) = 0;
        virtual void visit(ASTTempNode&) = 0;
        virtual void visit(ASTIdentNode&) = 0;
        virtual void visit(ASTIntConstNode&) = 0;

        virtual ~NodeVisitor() = default;
};

template<typename T>
class ParameterizedNodeVisitor {
    public:
        virtual T visit(ASTRootNode&) = 0;
        virtual T visit(ASTFunctionNode&) = 0;
        virtual T visit(ASTReturnNode&) = 0;
        virtual T visit(ASTTypeNode&) = 0;
        virtual T visit(ASTBinaryOpNode&) = 0;
        virtual T visit(ASTTempNode&) = 0;
        virtual T visit(ASTIdentNode&) = 0;
        virtual T visit(ASTIntConstNode&) = 0;

        virtual ~ParameterizedNodeVisitor() = default;
};

class ASTBuilderVisitor : public NodeVisitor {
    private:
        AST& ast;

        void erase_children(ASTNode&, AST_NODE_TYPE);
    public:
        ASTBuilderVisitor(AST& ast) : ast(ast) {}

        void visit(ASTRootNode&) override;
        void visit(ASTFunctionNode&) override;
        void visit(ASTReturnNode&) override;
        void visit(ASTTypeNode&) override;
        void visit(ASTBinaryOpNode&) override;
        void visit(ASTTempNode&) override;
        void visit(ASTIdentNode&) override;
        void visit(ASTIntConstNode&) override;

        ~ASTBuilderVisitor() = default;
};

class ASTPrinterVisitor : public NodeVisitor {
    private:
        AST& ast;
    public:
        ASTPrinterVisitor(AST& ast) : ast(ast) {}

        void visit(ASTRootNode&) override;
        void visit(ASTFunctionNode&) override;
        void visit(ASTReturnNode&) override;
        void visit(ASTTypeNode&) override;
        void visit(ASTBinaryOpNode&) override;
        void visit(ASTTempNode&) override;
        void visit(ASTIdentNode&) override;
        void visit(ASTIntConstNode&) override;

        ~ASTPrinterVisitor() = default;
};