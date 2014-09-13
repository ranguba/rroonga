# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

def use_cpu_nums
  proc_file = "/proc/cpuinfo"
  use_cpus = 1
  if File.exist?(proc_file)
    cpu_nums = `cat #{proc_file} |grep processor|wc -l`
  elsif RUBY_PLATFORM =~ /darwin/
    cpu_nums = `sysctl -n hw.ncpu`
  elsif RUBY_PLATFORM =~ /mswin|msys|mingw|cygwin|bccwin|wince|emc/
    cpu_nums = `echo %NUMBER_OF_PROCESSORS%`
  end
  use_cpus = cpu_nums.to_i / 2 if cpu_nums.to_i >=2
  use_cpus
end

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  config.vm.box = "ubuntu-14.04-x86_64"
  config.vm.box_url = "http://opscode-vm-bento.s3.amazonaws.com/vagrant/virtualbox/opscode_ubuntu-14.04_chef-provisionerless.box"

  config.vm.network "forwarded_port", guest: 80, host: 8080
  config.vm.network "private_network", ip: "192.168.33.107"
  config.ssh.forward_agent = true

  config.vm.provider("virtualbox") do |vb|
    vb.gui = false
    use_cpus = use_cpu_nums
    vb.customize ["modifyvm", :id, "--cpus", "#{use_cpus.to_s}"]
    vb.customize ["modifyvm", :id, "--memory", "2048"]
    vb.customize ["modifyvm", :id, "--ioapic", "on"]
    vb.customize ["modifyvm", :id, "--natdnshostresolver1", "on"]
    vb.customize ["modifyvm", :id, "--natdnsproxy1", "on"]
  end

  config.vm.provision :shell, path: "cross-build-rroonga.sh", privileged: false
end
