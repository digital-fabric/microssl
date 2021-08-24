require_relative './lib/microssl/version'

Gem::Specification.new do |s|
  s.name        = 'microssl'
  s.version     = MicroSSL::VERSION
  s.licenses    = ['MIT']
  s.summary     = 'Minimalistic OpenSSL library for Ruby'
  s.author      = 'Sharon Rosner'
  s.email       = 'sharon@noteflakes.com'
  s.files       = `git ls-files`.split
  s.homepage    = 'http://github.com/digital-fabric/microssl'
  s.metadata    = {
    "source_code_uri" => "https://github.com/digital-fabric/microssl"
  }
  s.rdoc_options = ["--title", "microssl", "--main", "README.md"]
  s.extra_rdoc_files = ["README.md"]
  s.extensions = ["ext/microssl/extconf.rb"]
  s.require_paths = ["lib"]
  s.required_ruby_version = '>= 2.6'

  s.add_development_dependency  'rake-compiler',        '1.1.1'
  s.add_development_dependency  'rake',               '~>13.0.6'
  s.add_development_dependency  'minitest',           '~>5.14.4'
end
