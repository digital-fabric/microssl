# frozen_string_literal: true

require_relative 'helper'
require 'microssl'

class MicroSSLTest < MiniTest::Test
  def test_security_level
    ctx = MicroSSL::Context.new
    
    assert_equal 2, ctx.security_level

    ctx.security_level = 0
    assert_equal 0, ctx.security_level
  end
end