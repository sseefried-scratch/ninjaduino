

require 'trigger'

describe Trigger do
  describe "thermometer" do
    before :each do
      @t = Trigger.new("thermometer", 20, 40)
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
      @t = Trigger.new("thermometer", 40, 20, lambda{|x| x})
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
  describe 'non-const transformer' do
    it 'should handle a transformer'
  end

  describe 'default button' do
    before :each do
      @t = Trigger.new("button")
    end

    it 'should fire' do
      @t.fire?(1010).should be_true
      @t.fire?(1010).should be_false
      @t.fire?(0).should be_false
      @t.fire?(1010).should be_true
    end
  end
  
  
end
