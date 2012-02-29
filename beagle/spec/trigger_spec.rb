require 'trigger'

describe Trigger do
  describe "thermometer" do
    before :each do
      @t = Trigger.new(20, 40, lambda{|x| x})
    end
    
    it "fires when it gets too hot" do
      @t.fire?(10).should == false
      @t.fire?(40).should == false
      @t.fire?(41).should == true
    end

    it "only fires once" do
      @t.fire?(41).should == true
      @t.fire?(41).should == false
    end

    it "doesn't reset at boundary" do
      @t.fire?(41).should == true
      @t.fire?(20).should == false
      @t.fire?(41).should == false
    end

    it "resets below boundary" do
      @t.fire?(41).should == true
      @t.fire?(19).should == false
      @t.fire?(41).should == true
    end
  end
end
