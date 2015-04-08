#!/usr/bin/env python

# genint - a tool to generate machine instructions interpreter
# for RISC ISA systems from their constraints definitions.
# Author: 2014 Grigory Rechistov grigory.rechistov@phystech.edu
#

# A machine instruction anatomy is split into individual fields:
# 31                                           0
# ----------------------------------------------
# |   f1    |f2 |f3|        f4      |    f5    |
# ----------------------------------------------
# There may be more than one set of fields with different boundaries between
# them, e.g.
# ----------------------------------------------
# |              g1           |       g2       |
# ----------------------------------------------
#
# An instruction is defined by specifying of four things:
# 1. Constraints for one or more of fields' values. Unconstrained fields are
# acting as variables - the instruction operands.
# 2. Mnemonics - how the instruction should be represented in disassembly.
# 3. Attributes - list of additional instruction parameters. 
# (e.g. 'branch' - states whether PC should be incremented (false) or not (true)) 
# 4. Semantics - what should be executed in order for an instruction to be
# correctly interpreted.
#
# The problem is to be able to tell one instruction from another in an efficient
# manner, as well as to extract its operands.
#

# TODO:
# 1. Actually implement and test handling of big-endian specifications.
# 2. Honor signedness of fields at their extraction
# 3. Logical fields - one can use others, so their extraction should be ordered
# 4. Find sa way to get rid of a common root field hack
# 5. Proper disassembly format for mnemonics

import sys
import copy
import re
import string
import argparse

def make_mask(s,e):
    w = e-s+1
    return ((1 << w) -1) << s

def nyi(s=""):
    print "Not yet implemented %s" % s

def check_token_as_name(s):
    if not re.match(r"[_a-zA-Z]\w*", s):
        raise Exception("Token '%s' is malformed - cannot be used as name" % s)

class Machine:
    def __init__(self, width, endianness = "LE", name = "machine"):
        self.name  = name
        check_token_as_name(name)
        self.width = width
        if self.width < 0:
            raise Exception("Negative width")
        self.endianness = endianness
        if self.endianness != "BE" and self.endianness != "LE":
            raise Exception("Unsupported endianness '%s'" % self.endianness)
        if   self.width > 64: raise Exception("Unsupported width >")
        elif self.width > 32: self.cast = "int64_t"
        elif self.width > 16: self.cast = "int32_t"
        else:                 self.cast = "int16_t"

    def __repr__(self):
        return self.name + str(self.width) + self.endianness

    def __cmp__(self, other):
        return self.name != other.name or \
               self.width != other.width or \
               self.endianness != other.endianness


class PhysicalField:
    ''' An actual field of an encoding of bit range [start, end] '''
    def __init__(self, machine, name, start, end, signed = False):
        self.machine    = machine
        self.name       = name
        self.start      = start
        self.end        = end
        self.signed     = signed
        check_token_as_name(name)
        if self.end >= machine.width:
            raise Exception("Bitfield end is past instruction width")
        if start > end:
            raise Exception("Bitfield start is more than its end")
        if start < 0:
            raise Exception("Bitfield start is negative")
        check_token_as_name(self.name)

        self.mask = make_mask(self.start, self.end)
        self.width = end - start +1
        if machine.width > 32: self.suffix = "ll"
        else: self.suffix = ""
        if not self.signed: self.suffix = "u" + self.suffix

    def __cmp__(self, other):
        return self.to_id() != other.to_id()

    def __repr__(self):
        ''' __repr__() has to represent all attributes of the field as we use it
            to find exact matches; __cmp__() reports only name clashes.
        '''
        if self.signed: sign = "-"
        else: sign = ""
        return "%s%s [%d:%d]" % (sign, self.name, self.start, self.end)

    def to_code(self):
        '''Convert to actual code representation'''
        return self.name

    def to_id(self):
        '''Return an identificator that can be used as part of a C name'''
        return self.name

    def to_extractor(self, value):
        '''Produce an expression that can be used to
        calculate this field from value'''
        if self.machine.endianness == "LE":
            return "((%s & %#x%s) >> %d)" % \
                (value, self.mask, self.suffix, self.start)
        else: # BE
            nyi()
            return "((%s & %#x%s) >> %d)" % \
                (value, self.mask, self.suffix, self.machine.width - self.start)

    def limits(self):
        '''return a tuple (min, max)'''
        maxuint = 1 << self.width
        if self.signed: return (-(1 + maxuint >> 1), maxuint >> 1)
        else:           return (0, maxuint)

class LogicalField(PhysicalField):
    '''A value that is calculated from other fields as a function'''
    def __init__(self, machine, name, width, fn, fieldlist, signed = False):
        if width <=0: raise Exception("Bad width: %d" % width)
        PhysicalField.__init__(self, machine, name, 0, width-1, signed)
        self.fn = fn
        self.fieldlist = fieldlist
        if len(self.fieldlist) == 0:
            raise Exception("Zero fields list")

    def __repr__(self):
        ''' __repr__() has to represent all attributes of the field as we use it
            to find exact matches; __cmp__() reports only name clashes.
        '''
        if self.signed: sign = "-"
        else: sign = ""
        return "%s%s (%s)[%d](%s)" % (sign, self.name, self.fn, self.width, self.fieldlist)


    def to_code(self):
        '''Convert to actual code representation'''
        return self.name
    def to_id(self):
        '''Give identificator that can be used as part of a C name'''
        return self.name

    def to_extractor(self, instr):
        '''Produce an expression that can be used to
        calculate this field from its operands'''
        # Note that actual instr value should not be used
        # - operands should be enough
        # Pass all fields to the function
        fldpattern = r"%s, " * len(self.fieldlist)
        pattern = r"%s("  + fldpattern + "decode_data)"
        namelist = [self.fn]
        for f in self.fieldlist: namelist.append(f) # .to_code()
        return  pattern % tuple(namelist)


class SimpleConstraint:
    '''Single condition of a form "field < const" '''
    def __init__(self, field, relation, constant):
        self.field = field
        self.relation = relation
        self.constant = constant

        limits = field.limits()
        if self.constant < limits[0] or self.constant > limits[1]:
            raise Exception("Constant is out of field's limits")
        if not (relation in ("<", "<=", ">", ">=", "!=", "==")):
            raise Exception("Unknown relation")

    def __repr__(self):
        return "%s %s %d" % (self.field.name, self.relation, self.constant)

    def to_code(self):
        return "(%s %s %d)" % \
            (self.field.to_code(), self.relation, self.constant)

    def get_fields(self):
        return [self.field]

    def to_id(self):
        rel_dict = {"<" : "l",
                    "<=": "le",
                    ">" : "g",
                    ">=": "ge",
                    "!=": "ne",
                    "==":  "eq" }
        return "%s_%s_%d" % (self.field.to_code, rel_dict[self.relation])

    def get_list(self):
        return [self]

    def __cmp__(self, other):
        # TODO optimize me
        return self.__repr__() != other.__repr__()

class FieldConstraint:
    ''' A set of simple relations for the same field connected with conjunctions
    (a < 3 && a > 4 && a != 5) '''
    def __init__(self, field = None, constraint = None):
        if field is None: self.field = constraint.field
        else:             self.field = field
        self.rel_list = []
        if constraint is not None:
            self.add_constraint(constraint)

    def __repr__(self):
        result = ""
        for r in self.rel_list:
            result += r.__repr__() + ", "
        return result

    def add_constraint(self, constraint):
        if constraint.field != self.field:
            raise Exception("Field mimatch: wanted %s, got %s" % \
                (self.field, constraint.field))
        self.rel_list.append(constraint)

    def to_code(self):
        if len(self.rel_list) == 0:
            raise Exception("Empty relations list")
        result = ""
        for f in self.rel_list[:-1]:
            result += f.to_code() + " && "
        result += self.rel_list[-1].to_code()
        return result

    def get_fields(self):
        return [self.field]

    def get_list(self):
        return [self]

    def __cmp__(self, other):
        # TODO optimize me
        return self.__repr__() != other.__repr__()

class ComplexConstraint:
    '''A set of constraints for multiple fields connected with conjunctions'''
    def __init__(self):
        self.rel_list = [] # will store FieldConstraint's indexed by Fields

    def __repr__(self):
        result = ""
        for r in self.rel_list:
            result += r.__repr__() + ", "
        return result

    def __cmp__(self, other):
        raise Exception("Cannot compare ComplexConstraint's")

    def add_constraint(self, constraint):
        fldcstrnt = None
        for f in self.rel_list:
            if constraint.field == f.field:
                fldcstrnt = f
                break
        if  fldcstrnt is not None:
            fldcstrnt.add_constraint(constraint)
        else:
            self.rel_list.append(FieldConstraint(field = None, constraint = constraint))

    def get_fields(self):
        key_list = []
        for k in self.rel_list:
            key_list.append(k.field)
        return key_list

    def to_code(self):
        if len(self.rel_list) == 0:
            raise Exception("Empty relation dict")
        result = ""
        for k in self.rel_list[:-1]:
            result += k.to_code() + " && "
        result     += self.rel_list[-1].to_code()
        return result

    def get_list(self):
        return self.rel_list


class FieldGroup:
    '''A set of physical and logical fields meant to be used in single instruction'''
    def __init__(self):
        # NOTE as we use dict, fields are stored in unsorted order, which is bad
        # because logical fields may depend from physical to be already caculated.
        # A solution here is to use a list instead.
        self.fields = dict()
        self.name = "Unnamed"

    def add_field(self, fld):
        if fld.name in self.fields.keys(): raise Exception("Field is already added")
        self.fields[fld.name] = fld

    def validate(self):
        '''Check fields overlapping and other things'''
        # make sure the machine is the same in all fields.
        if len(self.fields.values()) == 0:
            raise Exception("Empty FieldGroup")
        machine = self.fields.values()[0].machine
        for of in self.fields.values()[1:]:
            if of.machine != machine:
                raise Exception(\
                    "Machine specification mismatch: %s vs %s " %\
                    (of.machine, machine)
                )
        # Make sure that all bits are covered exactly once by all fields...
        mask = 0
        encountered_logical_fields = False # ...unless we have logical fields
        for f in self.fields.values():
            #print ">>> field: ", f
            if isinstance(f, LogicalField): # no checks for logical fields
                encountered_logical_fields = True
                continue
            if mask & f.mask:
                raise Exception(\
                    "Field %s (mask %#x) intersects with one of already defined: mask %#x" % \
                    (f, f.mask, mask)
                )
            mask = mask | f.mask
        # Check for undefined fields
        full_mask = (1 << machine.width) - 1
        undef_mask = full_mask & ~mask
        if undef_mask != 0 and not encountered_logical_fields:
            raise Exception(
                "The word's bits %#x are not covered in group %s" %\
                (undef_mask, self.name))


    def merge(self, other):
        if self.fields.values() and \
            (self.fields.values()[0].machine != other.get_fields()[0].machine):
            raise Exception("Machines do not match")
        # Operate with other as a list of fields because we cannot known whether
        # it is a dict or a single field.
        for of in other.get_fields():
            for sf in self.get_fields():
                if of == sf:
                    # We have a name clash; but maybe it's just the same field?
                    if of.__repr__() == sf.__repr__():
                        print "// >>> DEBUG: redeclaring the same field %s" % of.name
                    else: # mismatch in definitions
                        raise Exception("Double definition of field %s" % f.name)

        # TODO tag individual fields of other with FieldSet so that their origin
        # can still be established after merge
        for fld in other.get_fields():
            self.fields[fld.name] = fld

    def get_fields(self):
        return self.fields.values()

class Pattern:
    '''A pattern for an instruction'''
    def __init__(self, name, constraints, mnemonic = "", semantics = "", operands = None, attributes = None):
        self.name = name
        self.constraints = constraints
        self.mnemonic = mnemonic
        self.semantics = semantics
        self.operands = operands
        self.attributes = attributes

    def get_fields(self):
        return self.constraints.get_fields()

    def __repr__(self):
        return "<i:%s>" % self.name

    def to_code(self):
        return "/* instruction %s */\n%s;" % (self.name, self.semantics)

    def to_mnemonic(self):
        # TODO implement me
        return 'snprintf(res, 128, %s);' % self.mnemonic
        pass

class ParseTree:
    ''' A tree node to represent search sequence'''
    def __init__(self, constraint = None):
        #self.type = "node"
        self.nodes = []
        self.instr = None
        self.constraint = constraint

    #def __repr__(self):
        #return "[constraint: %s, instr: %s, nodes: %s]" % (self.constraint, self.instr, self.nodes)

    def find_node(self, cstrlist):
        ''' Find a node or create one that matches the cstrlist'''
        #print ">>> ---------------------------"
        #print ">>> find_node enter: cstrlist =", cstrlist
        #print ">>> find_node enter: self.constraint=", self.constraint

        if len(cstrlist) == 0:
            raise Exception("Failed to find/add the node")
        cur = cstrlist[0]
        if self.constraint is None:
            if len(self.nodes) != 0: raise Exception("Leaves of a free node must be absent")
            self.constraint = cur
        if len(cstrlist) == 1:
            if self.constraint != cur:
                for n in self.nodes:
                    if n.constraint == cur: # Found a leaf that matches
                        return n
                # Not found, create and return a new leaf
                print "// >>> find_node: new node with constraint = ", cur
                next_node = ParseTree(cur)
                self.nodes.append(next_node)
                return next_node
            else: # We are the node
                return self
        else: # There are more constraints
            if cur != self.constraint:
                #print ">>> cur:", cur
                #print ">>> self.constraint:", self.constraint
                raise Exception("Failed to match current node's constraint")
            nextcur = cstrlist[1]
            for n in self.nodes:
                if n.constraint == nextcur:
                    return n.find_node(cstrlist[1:])
            # Not found, create an empty node
            next_node = ParseTree(nextcur)
            self.nodes.append(next_node)
            return next_node.find_node(cstrlist[1:])

    def add_constraint(self, cstrlist, instr):
        leaf = self.find_node(cstrlist)
        #print ">>> leaf == ", leaf
        if len(leaf.nodes) != 0:
            raise Exception("Ambiguous constraints for %s" % instr.name)
        leaf.instr = instr


    def add_pattern(self, instr):
        # We add the root constraint to the head of the list
        cstrlist = [self.constraint] + instr.constraints.get_list()
        self.add_constraint(cstrlist, instr)

    def print_tree(self, shift = 1):
        padding = " " * shift
        if (self.instr is None) and (len(self.nodes) == 0):
            print padding, "Malformed node: %s" % self
            #raise Exception("Malformed node: %s" % self)
        print padding,
        #print self,
        if self.constraint is not None:
            print "Cnstr: ", self.constraint.to_code(),
        if self.instr is not None:
            print "Instr: ", self.instr,
        print
        for l in self.nodes:
            l.print_tree(shift + 4)
        if shift == 1:
            print "--------"

    def produce_semantics(self, instr, shift):
        ''' Generate semantics block with all parameters substituted'''
        padding = " " * shift
        # make lines aligned by inserting padding after every newline
        instr_text = \
            self.instr.to_code().replace("\n", "\n"+ padding)
        result = padding + instr_text + "\n"
        return result

    def produce_mnemonic(self, instr, shift):
        ''' Generate disassembly block with all parameters substituted'''
        padding = " " * shift
        # make lines aligned by inserting padding after every newline
        instr_text = \
            self.instr.to_mnemonic().replace("\n", "\n"+ padding)
        result = padding + instr_text + "\n"
        return result


    def gen_decode(self, shift = 0, last = True, disassembly = False):
        '''Return a code for decoding'''
        # TODO add extracting of fields and tracking of which fields are
        # visible in current context
        padding = " " * shift
        if not disassembly: fail_string = "return 0"
        else:               fail_string = 'snprintf(res, 128, "[illegal instruction] instr = 0x%x", instr)'
        result = ""
        result += padding + "if (%s) {\n" % self.constraint.to_code()
        if self.instr is not None:
            if not disassembly:
                result += padding + 'prologue(decode_data.cpu);\n'
                result += self.produce_semantics(self.instr, shift + 4)
                result += padding
                if self.instr.attributes['branch']:
                    result += 'branch_'
                result += 'epilogue(decode_data.cpu);\n'
            else:
                result += self.produce_mnemonic(self.instr, shift + 4)
        if len(self.nodes) != 0:
            for leaf in self.nodes[:-1]:
                result += leaf.gen_decode(shift +4, last=False, disassembly=disassembly)
            leaf = self.nodes[-1]
            result += leaf.gen_decode(shift +4, last=True, disassembly=disassembly) + "\n"
        result += padding + "} "

        if not last:
            result += "else \n"
        else:
            result += "else {\n"
            result += padding + "    " + fail_string + ";\n"
            result += padding + "} \n"
        return result


    # Some helper functions for nicer code generation
    def gen_decode_head(self, shift = 0, disassembly = False):
        ''' Produce a function header. Should be applied to the tree root'''
        result = ""
        padding = " " * shift
        machname = self.constraint.get_fields()[0].machine.name # too long...
        casttype = self.constraint.get_fields()[0].machine.cast
        if not disassembly:
            result += "/* Decode (and simulate) instr\n * Return 0 if failed to decode, 1 if success */\n"
            result += "int %s_decode(%s instr, %s_decode_data_t decode_data) { \n"%(
                        machname, casttype, machname)
        else:
            result += "/* Disassemble instr and return string */\n"
            result += "char* %s_disasm(%s instr, %s_decode_data_t decode_data) { \n"%(
                        machname, casttype, machname)
            result += padding + "char *res = malloc(128);\n"
            result += padding + "res[0] = 0;\n"
        return result

    def gen_decode_tail(self, shift = 0, disassembly = False):
        machname = self.constraint.get_fields()[0].machine.name # too long...
        padding = " " * shift
        result = ""
        if not disassembly:
            result += padding + "return 1;\n"
            result += "}; // %s_decode()\n" % (machname)
        else:
            result += padding + "return res;\n"
            result += "}; // %s_disasm()\n" % (machname)
        return result;


class InstructionSet:
    '''A group of Pattern's comprising the ISA'''
    def __init__(self, machine, fieldspec):
        self.machine = machine
        self.ilist = []
        self.root = ParseTree()
        # Deepcopy below is not needed if we get rid
        # of fieldspec mutation with globconstr
        self.fieldspec = copy.deepcopy(fieldspec)

        # This machine_word_field is artificial, but currently it's how it works.
        machine_word_field = PhysicalField(machine,
                                           machine.__repr__(),
                                           0,
                                           machine.width - 1)
        globconstr = \
            SimpleConstraint(machine_word_field, "<=", (1 << machine.width) -1)
        self.root.find_node([globconstr]) # Create a common root; yes, this is crap.
        self.fieldspec.merge(globconstr)

    def add_instruction(self, instr):
        self.ilist.append(instr)

    def sort_instructions(self):
        '''Shuffle constraints inside individual patterns.
           Apply any heuristics to optimize a parse tree to be built. '''
        # Nothing for now
        pass

    def build_decoder(self):
        for i in self.ilist:
            self.root.add_pattern(i)
            #self.root.print_tree()

    def gen_decode(self, shift = 4, disassembly = False):
        print "/* >>> DEBUG print tree "
        self.root.print_tree()
        print ">>> END DEBUG print tree */"

        padding = " " * shift
        result = ""
        result += self.root.gen_decode_head(shift = shift,
                                            disassembly = disassembly)
        result += self.gen_fields_extraction(shift = shift)
        result += self.root.gen_decode(shift = shift, disassembly = disassembly)
        result += self.root.gen_decode_tail(shift = shift,
                                            disassembly = disassembly)
        return result

    def gen_fields_extraction(self, shift = 4):
        ''' Create a string with all fields to be extracted at the beginning.
        NOTE: this is actually suboptimal, as not every fields is always
        needed to decode. This should be substituted with on-demand
        fields calculation at traverse of decode tree.
        '''
        padding = " " * shift
        result = ""
        for fld in self.fieldspec.get_fields():
            casttype = fld.machine.cast
            if not fld.signed: casttype = "u" + casttype
            result += padding + \
                     "UNUSED " + \
                     casttype + " " + \
                     fld.name + " = " + \
                     fld.to_extractor("instr") + ";\n"
        return result

def parse_constraint_string(s, fs):
    '''Take string s of form
    f1 > 3 && f2 < 4 && f2 == 4
    and build a constraint, based on knowledge of fields from fs: FieldGroup
    '''
    result = ComplexConstraint()
    # Remove spaces and split by &&
    chunks = s.replace(" ", "").split("&&")
    #print ">>> parse_constraint_string: chunks = ", chunks
    for chunk in chunks:
        # identificator, relation, number
        m = re.search(r'([_a-zA-Z]\w*)([\<\>\!\=]?\=?)(\w+)', chunk)
        if m:
            ident    = m.group(1)
            relation = m.group(2)
            constant = int(m.group(3), 0)
            #print ">>> match: ", ident, relation, constant
            if not (ident in fs.fields.keys()):
                raise Exception("%s is not a known field" % ident)
            fld = fs.fields[ident]
            result.add_constraint(SimpleConstraint(fld, relation, constant))
        else:
            raise Exception("Failed to parse chunk '%s'" % chunk)
    return result

def parse_field_string(s, machine, name = "Unnamed"):
    '''Take a string of a form
    f1<0:3>, -f2<4:3>, f3 <3>, g1<30>(give_g1:f1)
    and build a FieldGroup.
    '''
    result = FieldGroup()
    result.name = name
    chunks = s.replace(" ", "").split(",")
    for chunk in chunks:
        signed = False
        if chunk[0] == "-":
            signed = True
            chunk = chunk[1:]
        m = re.search(r"(\w+)\<([\d:]+)\>\(?([\w:]*)\)?", chunk)
        if m:
            name = m.group(1)
            check_token_as_name(name)
            bitspec = m.group(2).split(":")
            start = None
            end = None
            if len(bitspec) == 2:
                start = int(bitspec[0])
                end   = int(bitspec[1])
            elif len(bitspec) == 1:
                start = int(bitspec[0])
                end   = int(bitspec[0])
            else: raise Exception("Too many colons in bitspec (expected zero or one)")

            func = ""
            deplist = []
            func_items = m.group(3).split(":")
            if len(func_items) > 0:
                func = func_items[0]
                deplist = func_items[1:]

            fld = None
            if len(func) != 0: # there is a function specification - it's a logical field
                fld = LogicalField (machine, name, start, func, deplist, signed)
            else: # it's a physical field
                fld = PhysicalField(machine, name, start, end, signed)
            result.add_field(fld)
        else:
            raise Exception("Failed to parse chunk '%s'" % chunk)
    result.validate()
    return result

def make_default_attributes():
    return {'branch': False}

def parse_attributes_string(s):
    '''Takes a string of attributes
        and build an attribute dictionary in form
        {attribute_name: value}
    '''
    result = make_default_attributes()
    lines = s.split(",")
    for l in lines:
        if l.strip() in result:
            result[l.strip()] = True
        else:
            raise Exception("unknown attribute")
    return result

def parse_instruction(s, machine, fieldset):
    '''Take a set of strings in form:
    instruction: ADD_R_M
    operands: g1, s2
    pattern: f1 > 3 && f2 < 4 && f2 == 4
    mnemonic: "add %s, %s", f1, s2
    attributes: branch
    <arbitrary text for semantics>
    endinstruction
    - and build a Pattern
    '''
    lines = s.split("\n")
    #print ">>> parse_instruction: lines = ", lines

    name = None
    operands = None
    constraints = None #ComplexConstraint()
    mnemonic = None
    attributes = None
    semantics = ""

    for l in lines:
        l = l.strip(" ")
        #print ">>> stripped l: ", l
        if len(l) == 0: continue
        m = re.search(r'^instruction:\s*([^\s]+)', l)
        if m:
            name = m.group(1)
            check_token_as_name(name)
            continue
        m = re.search(r'^operands:\s*(.+)', l)
        if m:
            if operands is not None:
                raise Exception("operands are already defined")
            operands = m.group(1)
            # TODO split them into tokens
            continue
        m = re.search(r'^pattern:\s*(.+)', l)
        if m:
            constraints = parse_constraint_string(m.group(1), fieldset)
            continue
        m = re.search(r'^mnemonic:\s*(.+)', l)
        if m:
            mnemonic = m.group(1)
            continue
        m = re.search(r'^attributes:\s*(.+)', l)
        if m:
            if attributes is not None:
                raise Exception("attributes are already defined")
            attributes = parse_attributes_string(m.group(1))
            continue
        m = re.search(r'^endinstruction', l)
        if m: break
        # Not matched - must be semantics line
        semantics += l + '\n'

    if attributes is None:
        attributes = make_default_attributes()
    if name is None:
        raise Exception("The instruction definition does not specify name")
    if constraints is None:
        raise Exception("The instruction definition does not specify pattern")
    if mnemonic is None:
        raise Exception("The instruction definition does not specify mnemonic")
    if len(semantics) == 0:
        raise Exception("The instruction definition does not specify semantics")
    #print ">>> parse_instruction: ", (name, operands, constraints, mnemonic, semantics)
    return Pattern(name, constraints, mnemonic, semantics, operands, attributes)

def parse_specification(text):
    '''Return a tuple (machine, fields, instructions_text)
    of individual instructions from text'''
    result = []
    lines = text.split("\n")
    instr_def = False
    curdef = ""
    machine = None
    fg = FieldGroup()

    for l in lines:
        l = l.strip(" ")
        m = re.search(r'^machine:\s*(.*)', l)
        # A string with machine definition:
        # machine: name, width, LE|BE
        if m:
            tokens = m.group(1).split(",")
            if (len(tokens) != 3):
                raise Exception("Machine defnition: wrong number of parameters")
            if machine is not None:
                raise Exception("Machine has already been defined")
            machine = Machine(name       = tokens[0].strip(" "),
                              width      = int(tokens[1]),
                              endianness = tokens[2].strip(" "))
            continue
        m = re.search(r'^group:\s*([^,]+),(.*)', l)
        # A string with field group definition:
        # group: name, definition
        if m:
            name = m.group(1)
            defn = m.group(2)
            check_token_as_name(name)
            #print ">>> field group: name='%s', defn='%s'" % (name, defn)
            if machine is None:
                raise Exception("Machine is not defined, cannot parse groups")
            nfg = parse_field_string(defn, machine, name)
            fg.merge(nfg)
            continue
        m = re.search(r'^instruction:\s*([^\s]+)', l)
        if m:
            curdef += l + '\n'
            instr_def = True
            continue
        if not instr_def:
            # We are outside instruction: ... endinstruction block, ignore the line
            continue
        m = re.search(r'^endinstruction', l)
        if m:
            curdef += l + '+\n'
            result.append(curdef)
            curdef = ""
            instr_def = False
            continue
        curdef += l + '\n'
    if machine is None:
        raise Exception("Missing machine definition")

    return (machine, fg, result)

# ### main ###
def main():

    text = ""
    parser = argparse.ArgumentParser(description =\
        'Generate source code for decode, disassembly and simulation of \
        machine instruction set architecture')
    parser.add_argument('file', help='Input file with the specification')
    parser.add_argument('-o', '--output', default = None,
                        help='File name for the result')
    parser.add_argument('-p', '--prefix', default = None,
                        help='File to be prepended verbatim')
    parser.add_argument('-s', '--suffix', default = None,
                        help='File to be added to the end verbatim')
    #parser.add_argument
    args = parser.parse_args()
    with open (args.file, "r") as infile:
        text=infile.read()

    if args.output is None:
        cout = sys.stdout
    else:
        cout = open(args.output, "w")
        sys.stdout = cout # am I evil? Yes I am! all of 'print' will now be directed to file

    (machine, fieldgroup, instructions) = parse_specification(text)

    isa = InstructionSet(machine, fieldgroup)

    for i in instructions:
        isa.add_instruction(parse_instruction(i, machine, fieldgroup))

    isa.sort_instructions()
    isa.build_decoder()

    print "/* This file is autogenerated by %s from %s. Do not edit! */" % \
        (sys.argv[0], args.file)
    if args.prefix is not None:
        with open (args.prefix, "r") as prefix:
            text=prefix.read()
            print text
    print "// ---------------- decoder --------------------\n"

    result = ""
    result += isa.gen_decode()

    print result

    print "// ---------------- disassembly --------------------\n"
    result = ""
    result += isa.gen_decode(disassembly = True)
    print result

    if args.suffix is not None:
        with open (args.suffix, "r") as suffix:
            text=suffix.read()
            print text

    cout.close()
    return 0

if __name__ == "__main__":
    main()

