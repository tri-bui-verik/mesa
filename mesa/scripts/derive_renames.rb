#!/usr/bin/env ruby
# Copyright (c) 2004-2017 Microsemi Corporation "Microsemi".
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

require 'pp'
require_relative './hdr_parser.rb'

#parser = CParser.new
#pp parser.parse("""
#typedef struct {
#    BOOL                    enable;                  /**< Admin enable/disable */
#    BOOL                    fdx;                     /**< Forced duplex mode */
#    BOOL                    flow_control;            /**< Flow control (Standard 802.3x) */
#    BOOL                    pfc[VTSS_PRIOS];         /**< Priority Flow control (802.1Qbb)*/
#    vtss_port_speed_t       speed;                   /**< Forced port speed */
#    vtss_port_speed_fiber_t dual_media_fiber_speed;  /**< Speed for dual media fiber ports*/
#    u32 max_length;              /**< Max frame length */
#} vtss_port_custom_conf_t;
#""")

#pp ast

def sym_rename s
    if /^vtss_([a-zA-Z0-9_]*)/ =~ s
        puts "s/\\bvtss_#{$1}\\b/mesa_#{$1}/"
    elsif /^VTSS_([a-zA-Z0-9_]*)/ =~ s
        puts "s/\\bVTSS_#{$1}\\b/MESA_#{$1}/"
    else
        #puts "No match #{s}"
    end
end

def handle_extern_c ast
    return if ast[:keyword] != "extern"
    handle_root ast[:root]
end

def handle_typedef_enum ast
    return if ast[:enum_type_declare].nil?

    ast[:enum_type_declare][:enums].each do |e|
        next if e[:enum].nil?
        sym_rename e[:enum][:enum_name]
        #puts "Enum: #{e[:enum]}"
    end

end

def handle_typedef_struct_union ast
end

def handle_typedef ast
    return if ast[:keyword] != "typedef"
    return if ast[:typedef_name].nil?

    #puts "Type: #{ast[:typedef_name]}"
    sym_rename ast[:typedef_name]
    handle_typedef_enum ast
    handle_typedef_struct_union ast
end

def handle_func_proto ast
    return if ast[:func_proto].nil?
    n = ast[:func_proto][:normal]
    return if n.nil?
    sym_rename n[:name]
    #puts "Func: #{n[:name]}"
end

def handle_root ast
    ast.each do |e|
        handle_extern_c e
        handle_typedef e
        handle_func_proto e
    end
end

ARGV.each do |x|
    begin
        parser = CParser.new()
        ast = parser.parse(IO.binread(x), :reporter => Parslet::ErrorReporter::Deepest.new)
        handle_root ast[:root]
    rescue Parslet::ParseFailed => error
        puts error.cause.ascii_tree
    end
end


#ok = 0
#err = 0
#cnt = 0
#
#Dir.glob("/home/anielsen/work/webstax2/vtss_api/include/*api.h").each do |d|
#    res = nil
#    begin
#    parser = CParser.new
#    res = parser.parse(IO.binread(d))
#    rescue
#        puts "Failed #{d}"
#        err += 1
#    else
#        puts "OK #{d}"
#        ok += 1
#    ensure
#        cnt += 1
#    end
#end
#
#
#puts "#{ok}/#{cnt}"

