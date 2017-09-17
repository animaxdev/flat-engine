local FunctionalScriptNode = flat.require 'graph/script/functionalscriptnode'
local PinTypes = flat.require 'graph/pintypes'

local BoolNode = FunctionalScriptNode:inherit 'Bool'

function BoolNode:buildPins()
    self.boolOutPin = self:addOutputPin(PinTypes.BOOL, 'Bool')
end

function BoolNode:execute(runtime)
    runtime:writePin(self.boolOutPin, self.value)
end

function BoolNode:setValue(value)
    assert(type(value) == 'boolean')
    self.value = value
end

return BoolNode