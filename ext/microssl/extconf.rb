# frozen_string_literal: true

require 'rubygems'
require 'mkmf'

$CFLAGS << " -Wno-format-security"
CONFIG['optflags'] << ' -fno-strict-aliasing' unless RUBY_PLATFORM =~ /mswin/

dir_config 'microssl_ext'
create_makefile 'microssl_ext'
