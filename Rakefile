# frozen_string_literal: true

require "bundler/gem_tasks"
require "rake/clean"

require "rake/extensiontask"
Rake::ExtensionTask.new("microssl_ext") do |ext|
  ext.ext_dir = "ext/microssl"
end

task :recompile => [:clean, :compile]
task :default => [:compile, :test]

task :test do
  exec 'ruby test/test_microssl.rb'
end
