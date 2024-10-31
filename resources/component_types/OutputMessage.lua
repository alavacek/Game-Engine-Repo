OutputMessage = {
    message = "Hello world",

    OnStart = function(self)
        print(self.entity:GetName() .. " : " .. self.message)
    end,

    OnUpdate = function(self)
        print(self.entity:GetName() .. " : Update On Frame " .. Application.GetFrame())

        local mouseScroll = Input.GetMouseScrollDelta()
        print("Mouse scroll: " .. mouseScroll)
    end,

    OnLateUpdate = function(self)
        print(self.entity:GetName() .. " : Late Update")
    end
}
