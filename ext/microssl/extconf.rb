# frozen_string_literal: true

require 'rubygems'
require 'mkmf'

dir_config 'microssl_ext'

found_ssl = if pkg_config 'openssl'
  puts 'using OpenSSL pkgconfig (openssl.pc)'
  true
elsif %w'crypto libeay32'.find {|crypto| have_library(crypto, 'BIO_read')} &&
    %w'ssl ssleay32'.find {|ssl| have_library(ssl, 'SSL_CTX_new')}
  true
else
  false
end

if !found_ssl
  puts '** No OpenSSL lib found'
  exit!
end

have_header "openssl/bio.h"

# below is  yes for 1.0.2 & later
have_func  "DTLS_method"                           , "openssl/ssl.h"

# below are yes for 1.1.0 & later
have_func  "TLS_server_method"                     , "openssl/ssl.h"
have_func  "SSL_CTX_set_min_proto_version(NULL, 0)", "openssl/ssl.h"

have_func  "X509_STORE_up_ref"
have_func("SSL_CTX_set_ecdh_auto(NULL, 0)", "openssl/ssl.h")

have_func("SSL_CTX_get_security_level")

# Random.bytes available in Ruby 2.5 and later, Random::DEFAULT deprecated in 3.0
if Random.respond_to?(:bytes)
  $defs.push("-DHAVE_RANDOM_BYTES")
  puts "checking for Random.bytes... yes"
else
  puts "checking for Random.bytes... no"
end

create_makefile 'microssl_ext'

