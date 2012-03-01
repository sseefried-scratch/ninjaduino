require 'spec_helper'
describe "sanity" do
  it "should load if it's an rb file" do
    # executed for exceptiony side effect
    Dir.glob("**/*.rb").each do |file|
      next if file =~ /ninja_driver/
      next if file =~ /^spec/
      load file
    end

  end
end

