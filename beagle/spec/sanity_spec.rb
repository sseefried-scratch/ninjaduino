require 'spec_helper'
describe "sanity" do
  it "should load if it's an rb file" do
    # executed for exceptiony side effect
    Dir.glob("**/*.rb").each do |file|
      load file
    end

  end
end

