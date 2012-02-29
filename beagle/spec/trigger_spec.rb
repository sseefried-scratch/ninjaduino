

require 'trigger'

describe Trigger do
  describe "thermometer" do
    before :each do
      @t = Trigger.new(20, 40, lambda{|x| x})
    end
    
    it "fires when it gets too hot" do
      @t.fire?(10).should be_false
      @t.fire?(40).should be_false
      @t.fire?(41).should be_true
    end

    it "only fires once" do
      @t.fire?(41).should be_true
      @t.fire?(41).should be_false
    end

    it "doesn't reset at boundary" do
      @t.fire?(41).should be_true
      @t.fire?(20).should be_false
      @t.fire?(41).should be_false
    end

    it "resets below boundary" do
      @t.fire?(41).should be_true
      @t.fire?(19).should be_false
      @t.fire?(41).should be_true
    end

  end
  describe "inverted thermometer" do
    before :each do
      @t = Trigger.new(40, 20, lambda{|x| x})
    end

    it 'fires when it gets cold enough' do
      @t.fire?(19).should be_true
    end

    it 'resets above boundary' do
      @t.fire?(19).should be_true
      @t.fire?(19).should be_false
      @t.fire?(41).should be_false
      @t.fire?(19).should be_true
    end
    
  end

  
end
