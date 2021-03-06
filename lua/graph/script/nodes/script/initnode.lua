local ScriptNode = flat.require 'graph/script/scriptnode'
local PinTypes = flat.require 'graph/pintypes'

local InitNode = ScriptNode:inherit 'Init'

function InitNode:buildPins()
    self.impulseOutPin = self:addOutputPin(PinTypes.IMPULSE, 'Out')
end

function InitNode:execute(runtime)
    runtime:impulse(self.impulseOutPin)
end

function InitNode:addedToGraph(graph)
    graph:addEntryNode(self)
end

return InitNode