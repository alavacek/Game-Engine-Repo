OutputMessage = {
    message = "Hello world",

    OnStart = function(self)
        print(self.entity:GetName() .. " : " .. self.message)

        Application.OpenURL("https://alavacek.github.io/")
        Application.Sleep(10000)
    end,

    OnUpdate = function(self)
        print(self.entity:GetName() .. " : Update On Frame " .. Application.GetFrame())
    end,

    OnLateUpdate = function(self)
        print(self.entity:GetName() .. " : Late Update")
    end
}
