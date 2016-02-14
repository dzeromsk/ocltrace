#!/usr/bin/env python

# TODO(dzeromsk): Fix issue with function pointer parameter

import sys
from pycparser import c_parser, c_ast, parse_file, c_generator

class EmitGenerator(object):
    def _generate_type(self, n, modifiers=[]):

        typ = type(n)

        if typ == c_ast.TypeDecl:
            s = ''
            nstr = ""
            #nstr += "emit_start(\"%s\");" % n.declname if n.declname else ''
            mmod = ""
            mmod2 = ""
            for i, modifier in enumerate(modifiers):
                if isinstance(modifier, c_ast.ParamList):
                    if (i+1 < len(modifiers) and isinstance(modifiers[i + 1], c_ast.PtrDecl)):
                        mmod = '_ptr'
                        mmod2 = '*'
                    for j, p in enumerate(modifier.params):
                        if n == p.type or n == p.type.type:
                            t = "_".join(n.type.names)
                            #nstr += "emit_%s%s(_%d);" % (t, mmod, j)
                            nstr += "inline void emit_%s%s(const %s%s x) { }" % (t, mmod, t, mmod2)

                if isinstance(modifier, c_ast.FuncDecl):
                    nstr += self.visit(modifier.args)

            if nstr: 
                s += nstr
            return s
        elif typ == c_ast.Decl:
            return self._generate_type(n.type)
        elif typ == c_ast.Typename:
            return self._generate_type(n.type, modifiers)
        elif typ in (c_ast.ArrayDecl, c_ast.PtrDecl, c_ast.FuncDecl):
            return self._generate_type(n.type, modifiers + [n])
        else:
            return self.visit(n)

    def visit(self, node):
        method = 'visit_' + node.__class__.__name__
        return getattr(self, method, self.generic_visit)(node)

    def generic_visit(self, node):
        return ''
    
    def visit_Decl(self, n):
        return self._generate_type(n)

    def visit_ParamList(self, n):
        return "\n" + '\n'.join(self._generate_type(param, [n]) for param in n.params)


def funcDecl(node):
    emitgen = EmitGenerator()
    emit = emitgen.visit(node)
    print emit
    print

class DeclVisitor(c_ast.NodeVisitor):
    def visit_Decl(self, node):
        if isinstance(node.type, c_ast.FuncDecl):
            funcDecl(node)


def show_func_defs(filename):
    ast = parse_file(filename, use_cpp=False)
    v = DeclVisitor()
    v.visit(ast)


if __name__ == "__main__":
    show_func_defs(sys.argv[1])

