OutputMessage = {
    message = "I am OutputMessage",

    OnStart = function(self)
        --print(self.entity:GetName() .. " OutputMessage Start" .. " : " .. self.message)
        -- self.entityRef = Entity.Instantiate("PlayerController")   
        --Scene.DontDestroy(self.entityRef)

        Event.Subscribe("OutMessage", self, self.EventOutMessage)
        --Event.Unsubscribe("OutMessage", self, self.EventOutMessage)

    end,

    OnUpdate = function(self)
        --print(self.entity:GetName() .. " OutputMessage Update " .. " : Update On Frame " .. Application.GetFrame())  
        -- Event.Unsubscribe("OutMessage", self, EventOutMessage)
    end,

    OnLateUpdate = function(self)
        --print(self.entity:GetName() .. " OutputMessage Late Update " .. " : Late Update")
    end,

    EventOutMessage = function(self, event)
        print("Event Called")
        event.SomeFunction()
    end
}
