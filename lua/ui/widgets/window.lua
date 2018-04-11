local Theme = flat.ui.settings.theme.window

local Window = {}
Window.__index = Window

function Window:new(parent)
    assert(parent)
    local o = setmetatable({
        window = nil,
        titleLabel = nil,
        toolbarsContainer = nil,
        contentContainer = nil,
        userContentContainer = nil,
        content = nil,

        bottomRightResizeIcon = nil,

        onClose = flat.Slot:new()
    }, self)
    o:build(parent)
    return o
end

function Window:close()
    self:onClose()
    self.window:removeFromParent()
end

function Window:setTitle(title)
    self.titleLabel:setText(title)
end

function Window:addToolbar()
    return flat.ui.Toolbar:new(self.toolbarsContainer)
end

function Window:setContent(content)
    self.userContentContainer:removeAllChildren()
    self.userContentContainer:addChild(content)
    self.content = content
end

function Window:getContentContainer()
    return self.userContentContainer
end

function Window:getContent()
    return self.content
end

function Window:build(parent)
    local parentWidth, parentHeight = parent:getComputedSize()
    local window = Widget.makeFixedSize(parentWidth - 100, parentHeight - 100)
    window:setPosition(50, -50)
    window:setBackgroundColor(Theme.BORDER_COLOR)

    do
        local windowContent = Widget.makeColumnFlow()
        windowContent:setMargin(1)
        windowContent:setSizePolicy(Widget.SizePolicy.EXPAND)

        -- title
        do
            local titleContainer = Widget.makeLineFlow()
            titleContainer:setSizePolicy(Widget.SizePolicy.EXPAND_X + Widget.SizePolicy.COMPRESS_Y)
            titleContainer:setPadding(4)
            titleContainer:setBackgroundColor(Theme.TITLE_BACKGROUND_COLOR)

            do
                local titleLabel = Widget.makeText('Window', table.unpack(Theme.TITLE_FONT))
                titleLabel:setSizePolicy(Widget.SizePolicy.EXPAND_X + Widget.SizePolicy.FIXED_Y)
                titleLabel:setTextColor(Theme.TITLE_TEXT_COLOR)
                titleLabel:setMargin(3, 0, 0, 3)
                titleLabel:mouseDown(function()
                    window:drag()
                end)
                titleLabel:mouseUp(function()
                    window:drop()
                end)

                titleContainer:addChild(titleLabel)
                self.titleLabel = titleLabel
            end

            do
                local closeWindowButton = Widget.makeImage(flat.assetPath 'ui/window/close-button.png')
                closeWindowButton:setMargin(2, 2, 0, 0)
                closeWindowButton:click(function()
                    self:close()
                end)
                titleContainer:addChild(closeWindowButton)
            end

            windowContent:addChild(titleContainer)
        end

        -- toolbars
        do
            local toolbarsContainer = Widget.makeColumnFlow()
            toolbarsContainer:setSizePolicy(Widget.SizePolicy.EXPAND_X + Widget.SizePolicy.COMPRESS_Y)
            windowContent:addChild(toolbarsContainer)
            self.toolbarsContainer = toolbarsContainer
        end

        -- user content
        do
            local contentContainer = Widget.makeExpand()

            do
                local userContentContainer = Widget.makeExpand()
                userContentContainer:setBackgroundColor(Theme.CONTENT_BACKGROUND_COLOR)
                contentContainer:addChild(userContentContainer)
                self.userContentContainer = userContentContainer
            end

            do
                local bottomRightResizeIcon = Widget.makeImage(flat.assetPath 'ui/window/bottom-right-resize.png')
                bottomRightResizeIcon:setPositionPolicy(Widget.PositionPolicy.BOTTOM_RIGHT)

                do
                    local initialWindowWidth, initialWindowHeight
                    local initialMouseX, initialMouseY
                    local resizeTimer
                    local mouseOver = false

                    local function resize()
                        local mouseX, mouseY = Mouse.getPosition()
                        local newWidth = initialWindowWidth + mouseX - initialMouseX
                        local newHeight = initialWindowHeight + initialMouseY - mouseY
                        newWidth = math.max(100, newWidth)
                        newHeight = math.max(100, newHeight)
                        window:setSize(newWidth, newHeight)
                    end
                    local function startResize()
                        Mouse.setCursor(Mouse.Cursor.SIZENWSE)
                        initialWindowWidth, initialWindowHeight = window:getSize()
                        initialMouseX, initialMouseY = Mouse.getPosition()
                        resizeTimer = Timer.new()
                        resizeTimer:onEnd(resize)
                        resizeTimer:start(0, true)
                    end
                    local function stopResize()
                        if not mouseOver then
                            Mouse.setDefaultCursor()
                        end
                        resize()
                        resizeTimer:stop()
                        resizeTimer = nil
                    end

                    bottomRightResizeIcon:mouseDown(function()
                        startResize()
                    end)
                    bottomRightResizeIcon:mouseUp(function()
                        stopResize()
                    end)
                    bottomRightResizeIcon:mouseEnter(function()
                        mouseOver = true
                        Mouse.setCursor(Mouse.Cursor.SIZENWSE)
                    end)
                    bottomRightResizeIcon:mouseLeave(function()
                        mouseOver = false
                        if not resizeTimer then
                            Mouse.setDefaultCursor()
                        end
                    end)
                end
        
                contentContainer:addChild(bottomRightResizeIcon)
                self.bottomRightResizeIcon = bottomRightResizeIcon
            end

            windowContent:addChild(contentContainer)
            self.contentContainer = contentContainer
        end

        window:addChild(windowContent)
    end

    parent:addChild(window)
    self.window = window
end

function Window:setSize(width, height)
    self.window:setSize(width, height)
end

function Window:setPosition(x, y)
    self.window:setPosition(x, y)
end

function Window:setPositionPolicy(positionPolicy)
    self.window:setPositionPolicy(positionPolicy)
end

function Window:setResizable(resizable)
    self.bottomRightResizeIcon:setVisible(resizable)
end

flat.ui.Window = Window