<!DOCTYPE TextureSet>
<TextureSet>
   <Nodes>
      <Node id="1" name="Node 1">
         <pos y="-76.9614" x="-736.946"/>
         <generator name="Perlin noise"/>
         <Settings>
            <setting id="color" value="#ffffff" type="QColor"/>
            <setting id="numoctaves" value="25" type="int"/>
            <setting id="persistence" value="0.7999999999999999" type="double"/>
            <setting id="randomizer" value="500" type="double"/>
            <setting id="zoom" value="40.36" type="double"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="2" name="Node 2">
         <pos y="-416.099" x="558.279"/>
         <generator name="Bricks"/>
         <Settings>
            <setting id="brickheight" value="38" type="int"/>
            <setting id="brickwidth" value="101" type="int"/>
            <setting id="color" value="#3f3f3f" type="QColor"/>
            <setting id="linewidth" value="10" type="int"/>
            <setting id="offsetx" value="0" type="int"/>
            <setting id="offsety" value="11" type="int"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="3" name="Node 3">
         <pos y="150.739" x="56.9663"/>
         <generator name="Fill"/>
         <Settings>
            <setting id="color" value="#7dff1b27" type="QColor"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="4" name="Node 4">
         <pos y="488.387" x="-582.722"/>
         <generator name="Blending"/>
         <Settings>
            <setting id="alpha" value="100" type="double"/>
            <setting id="mode" value="Normal" type="QString"/>
            <setting id="order" value="Slot 2 on top of Slot 1" type="QString"/>
         </Settings>
         <Sources>
            <source slot="0" source="1"/>
            <source slot="1" source="3"/>
         </Sources>
      </Node>
      <Node id="5" name="Node 5">
         <pos y="31.1681" x="1108.88"/>
         <generator name="Sine transform"/>
         <Settings>
            <setting id="amplitudeone" value="0.10000000000000003" type="double"/>
            <setting id="amplitudetwo" value="0.30000000000000004" type="double"/>
            <setting id="angle" value="0" type="double"/>
            <setting id="frequencyone" value="40" type="double"/>
            <setting id="frequencytwo" value="20" type="double"/>
            <setting id="offsetone" value="0" type="double"/>
            <setting id="offsettwo" value="150.97" type="double"/>
         </Settings>
         <Sources>
            <source slot="0" source="2"/>
         </Sources>
      </Node>
      <Node id="6" name="Node 6">
         <pos y="788.295" x="0.925557"/>
         <generator name="Blending"/>
         <Settings>
            <setting id="alpha" value="100" type="double"/>
            <setting id="mode" value="Normal" type="QString"/>
            <setting id="order" value="Slot 2 on top of Slot 1" type="QString"/>
         </Settings>
         <Sources>
            <source slot="0" source="4"/>
            <source slot="1" source="8"/>
         </Sources>
      </Node>
      <Node id="7" name="Node 7">
         <pos y="815.076" x="1265.49"/>
         <generator name="Perlin noise"/>
         <Settings>
            <setting id="color" value="#d8d6e7" type="QColor"/>
            <setting id="numoctaves" value="10" type="int"/>
            <setting id="persistence" value="0.7999999999999999" type="double"/>
            <setting id="randomizer" value="500" type="double"/>
            <setting id="zoom" value="10" type="double"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="8" name="Node 8">
         <pos y="609.741" x="625.483"/>
         <generator name="Set channels"/>
         <Settings>
            <setting id="channelAlpha" value="Second's alpha" type="QString"/>
            <setting id="channelBlue" value="First's blue" type="QString"/>
            <setting id="channelGreen" value="First's green" type="QString"/>
            <setting id="channelRed" value="First's red" type="QString"/>
         </Settings>
         <Sources>
            <source slot="0" source="7"/>
            <source slot="1" source="5"/>
         </Sources>
      </Node>
      <Node id="9" name="Node 9">
         <pos y="1273.34" x="-765.701"/>
         <generator name="Text"/>
         <Settings>
            <setting id="antialiasing" value="true" type="bool"/>
            <setting id="color" value="#ff0000" type="QColor"/>
            <setting id="fontname" value="AnyStyle" type="QString"/>
            <setting id="fontsize" value="33.97" type="double"/>
            <setting id="offsetleft" value="0" type="double"/>
            <setting id="offsettop" value="0" type="double"/>
            <setting id="rotation" value="0" type="double"/>
            <setting id="text" value="Text" type="QString"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="10" name="Node 10">
         <pos y="1440.73" x="-139.531"/>
         <generator name="Glow"/>
         <Settings>
            <setting id="color" value="#ffff00" type="QColor"/>
            <setting id="cutoutx" value="95" type="double"/>
            <setting id="cutouty" value="95" type="double"/>
            <setting id="firstblurlevel" value="3" type="double"/>
            <setting id="includesource" value="" type=""/>
            <setting id="mode" value="Multiply" type="QString"/>
            <setting id="ontop" value="" type=""/>
            <setting id="secondblurlevel" value="3" type="double"/>
            <setting id="size" value="4" type="double"/>
         </Settings>
         <Sources>
            <source slot="0" source="9"/>
         </Sources>
      </Node>
      <Node id="13" name="Node 13">
         <pos y="1550.21" x="504.719"/>
         <generator name="Blending"/>
         <Settings>
            <setting id="alpha" value="100" type="double"/>
            <setting id="mode" value="Normal" type="QString"/>
            <setting id="order" value="Slot 2 on top of Slot 1" type="QString"/>
         </Settings>
         <Sources>
            <source slot="0" source="6"/>
            <source slot="1" source="10"/>
         </Sources>
      </Node>
   </Nodes>
</TextureSet>
