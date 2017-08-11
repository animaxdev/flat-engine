local ScriptNode = flat.require 'graph/script/scriptnode'
local PinTypes = flat.require 'graph/pintypes'

local TestNode = ScriptNode:inherit()

function TestNode:getName()
    return 'Test'
end

function TestNode:buildPins()
    self.numberInPin = self:addInputPin(PinTypes.NUMBER, 'Number')
    self.stringInPin = self:addInputPin(PinTypes.STRING, 'String')
    self.impulseInPin = self:addInputPin(PinTypes.IMPULSE, 'In')

    self.numberOutPin = self:addOutputPin(PinTypes.NUMBER, 'Number')
    self.stringOutPin = self:addOutputPin(PinTypes.STRING, 'String')
    self.impulseOutPin = self:addOutputPin(PinTypes.IMPULSE, 'Out')
end

function TestNode:execute(runtime, inputPin)
    assert(inputPin == self.impulseInPin)
    local number = runtime:readPin(self.numberInPin)
    local string = runtime:readPin(self.stringInPin)

    print('number: ' .. tostring(number))
    print('string: ' .. tostring(string))

    runtime:writePin(self.numberOutPin, number)
    runtime:writePin(self.stringOutPin, string)
    runtime:impulse(self.impulseOutPin)
end

return TestNode