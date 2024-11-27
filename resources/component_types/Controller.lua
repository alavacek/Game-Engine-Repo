Controller = {
    OnStart = function(self)
        print(self.entity:GetName() .. " Controller Start")
        -- Audio.Play(0, "title_music", true)
        -- self.count = 0
        self.x = 0
        self.y = 0
        self.rigidbody = self.entity:GetComponent("Rigidbody")
    end,

    OnUpdate = function(self)
        print(self.entity:GetName() .." Controller Update " ..  " : Update On Frame " .. Application.GetFrame())     
        -- Text.Draw("OutputMessage2", 25, 25, "OpenSans-Regular", 16, 255, 255, 255, 255)
        if (Input.GetKey("w"))
        then
           Camera.SetPosition(Camera.GetPositionX(), Camera.GetPositionY() + 0.05)      
        elseif (Input.GetKey("a"))
        then
             
        elseif (Input.GetKey("s"))
        then
            Camera.SetPosition(Camera.GetPositionX(), Camera.GetPositionY() - 0.05)
        elseif (Input.GetKey("d"))
        then
            
        elseif (Input.GetKey("space"))
        then
            self.rigidbody:AddForce(Vector2(0, -100))
        elseif (Input.GetKeyDown("tab"))
        then
            self.rigidbody:SetPosition(Vector2(-3, 0)) 
        end

        
        Image.DrawEx("cat", self.rigidbody:GetPosition().x, self.rigidbody:GetPosition().y, self.rigidbody:GetRotation(), 1, 1, 0.5, 0.5, 255, 255, 255, 255, 0)
        
          
        -- Image.DrawPixel(400, 400, 255, 255, 255, 120)
        -- Image.DrawPixel(400, 405, 255, 0, 0, 255)
    end
}
