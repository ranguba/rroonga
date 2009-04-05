require 'fileutils'
require 'pathname'

require 'groonga'

module GroongaTestUtils
  class << self
    def included(base)
      base.setup :setup_tmp_directory
      base.teardown :teardown_tmp_directory
    end
  end

  def setup_tmp_directory
    @tmp_dir = Pathname(File.dirname(__FILE__)) + "tmp"
    FileUtils.rm_rf(@tmp_dir.to_s)
    FileUtils.mkdir_p(@tmp_dir.to_s)
  end

  def teardown_tmp_directory
    FileUtils.rm_rf(@tmp_dir.to_s)
  end
end
