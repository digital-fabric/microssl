# frozen_string_literal: true

require_relative 'helper'
require 'microssl'

class MicroSSLTest < MiniTest::Test
  def test_security_level
    ctx = MicroSSL::Context.new

    # assert_equal 2, ctx.security_level

    ctx.security_level = 0
    assert_equal 0, ctx.security_level

    ctx.security_level = 2
    assert_equal 2, ctx.security_level
  end

  def test_openssl_version_consts
    assert MicroSSL::OPENSSL_VERSION
    assert MicroSSL::OPENSSL_LIBRARY_VERSION
    assert MicroSSL.const_defined?(:OPENSSL_NO_SSL3)
    assert MicroSSL.const_defined?(:OPENSSL_NO_TLS1)
    assert MicroSSL.const_defined?(:OPENSSL_NO_TLS1_1)
  end
end