OutputMessage = {
    message = "Hello world",

    OnStart = function(self)
        print(self.entity:GetName() .. " : " .. self.message)
    end,

    OnUpdate = function(self)
        print(self.entity:GetName() .. " : Update")
    end,

    OnLateUpdate = function(self)
        print(self.entity:GetName() .. " : Late Update")
    end
}
