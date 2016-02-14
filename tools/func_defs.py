#!/usr/bin/env python

# TODO(dzeromsk): Fix issue with function pointer parameter

import sys
import re
from pycparser import c_parser, c_ast, parse_file, c_generator

def uconvert(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).upper()

def lconvert(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

class DeclarationGenerator(object):
    def _generate_type(self, n, modifiers=[]):

        typ = type(n)

        if typ == c_ast.TypeDecl:
            s = ''
            if n.quals: s += ' '.join(n.quals) + ' '
            s += self.visit(n.type)

            nstr = " " + n.declname if n.declname else ''

            for i, modifier in enumerate(modifiers):
                if isinstance(modifier, c_ast.ParamList):
                    if not nstr:
                        for i, p in enumerate(modifier.params):
                            if n == p.type or n == p.type.type:
                                nstr = " _%d" % i 

                if isinstance(modifier, c_ast.ArrayDecl):
                    if (i != 0 and isinstance(modifiers[i - 1], c_ast.PtrDecl)):
                        nstr = '(' + nstr + ')'
                    nstr += '[' + self.visit(modifier.dim) + ']'
                elif isinstance(modifier, c_ast.FuncDecl):
                    if (i != 0 and isinstance(modifiers[i - 1], c_ast.PtrDecl)):
                        nstr = '(' + nstr + ')'
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


class CallGenerator(object):
    def _generate_type(self, n, modifiers=[]):
        typ = type(n)

        if typ == c_ast.TypeDecl:
            s = ''
            nstr = "((%s)f[%s])" % (lconvert(n.declname), uconvert(n.declname)) if n.declname else ''

            for i, modifier in enumerate(modifiers):
                if isinstance(modifier, c_ast.ParamList):
                    if not nstr:
                        for i, p in enumerate(modifier.params):
                            if n == p.type or n == p.type.type:
                                nstr = "_%d" % i 

                if isinstance(modifier, c_ast.FuncDecl):
                    nstr += '(' + self._generate_type(modifier.args, modifiers + [n]) + ');'

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

    def visit_ParamList(self, n):
        return ', '.join(self._generate_type(param, [n]) for param in n.params)

class EmitGenerator(object):
    def _generate_type(self, n, modifiers=[]):

        typ = type(n)

        if typ == c_ast.TypeDecl:
            s = ''
            nstr = ""
            nstr += "emit_open(\"%s\");" % n.declname if n.declname else ''
            mmod = ""

            for i, modifier in enumerate(modifiers):
                if isinstance(modifier, c_ast.ParamList):
                    if (i+1 < len(modifiers) and isinstance(modifiers[i + 1], c_ast.PtrDecl)):
                        mmod = '_ptr'
                    for j, p in enumerate(modifier.params):
                        if n == p.type or n == p.type.type:
                            if j > 0 and j < len(modifier.params):
                                nstr += "emit_separator();\n"
                            t = "_".join(n.type.names)
                            nstr += "emit_%s%s(_%d);" % (t, mmod, j)

                if isinstance(modifier, c_ast.FuncDecl):
                    nstr += self.visit(modifier.args)
                    nstr += "\nemit_close();"

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
    declgen = DeclarationGenerator()
    emitgen = EmitGenerator()
    callgen = CallGenerator()

    decl = declgen.visit(node)
    d = decl.split()
    print decl
    print "{"
    emit = emitgen.visit(node)
    call = callgen.visit(node)
    l = emit.splitlines()
    if "cl_int_ptr" in l[-2]:
        print "\n".join(l[:-2])
        print "emit_flush();"
        print decl.split()[0] + " ret =", call
        print "\n".join(l[-2:])
    else:
        print emit
        print decl.split()[0] + " ret =", call

    print "emit_ret();"
    print "emit_%s(ret);" % d[0].replace("*", "_ptr")
    print "emit_end();"
    print "return ret;"
    print "}"
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

