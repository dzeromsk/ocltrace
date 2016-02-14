#!/usr/bin/env python

# TODO(dzeromsk): Fix issue with function pointer parameter

import sys
import re
from pycparser import c_parser, c_ast, parse_file, c_generator

class DeclarationGenerator(object):
    def _generate_type(self, n, modifiers=[]):

        typ = type(n)

        if typ == c_ast.TypeDecl:
            s = ''
            if n.quals: s += ' '.join(n.quals) + ' '
            s += self.visit(n.type)

            nstr = " " + lconvert(n.declname) if n.declname else ''

            for i, modifier in enumerate(modifiers):
                if isinstance(modifier, c_ast.ArrayDecl):
                    if (i != 0 and isinstance(modifiers[i - 1], c_ast.PtrDecl)):
                        nstr = '(' + nstr + ')'
                    nstr += '[' + self.visit(modifier.dim) + ']'
                elif isinstance(modifier, c_ast.FuncDecl):
                    nstr = ' (*' + nstr + ')'
                    nstr += '(' + self.visit(modifier.args) + ')'
                elif isinstance(modifier, c_ast.PtrDecl):
                    if modifier.quals:
                        nstr = '* %s %s' % (' '.join(modifier.quals), nstr)
                    else:
                        nstr = '*' + nstr

            if nstr: 
                s += nstr
            return s
        elif typ == c_ast.Decl:
            return self._generate_type(n.type)
        elif typ == c_ast.Typename:
            return self._generate_type(n.type, modifiers)
        elif typ == c_ast.IdentifierType:
            return ' '.join(n.names)
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
    
    def visit_IdentifierType(self, n):
        return ' '.join(n.names)

    def visit_ParamList(self, n):
        return ', '.join(self._generate_type(param, [n]) for param in n.params)


def uconvert(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).upper()

def lconvert(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

def funcDecl(node):
    # declname
    if isinstance(node.type.type, c_ast.TypeDecl):
        x = node.type.type.declname
    else:
        x = node.type.type.type.declname

    g = DeclarationGenerator()
    #print "typedef", g.visit(node)
    print "F(%s, %s)" % (uconvert(x), x)


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

