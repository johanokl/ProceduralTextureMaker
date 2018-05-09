<!DOCTYPE TextureSet>
<TextureSet>
   <Nodes>
      <Node id="1" name="Node 1">
         <pos x="-882.1046879370018" y="-907.4101954601308"/>
         <generator name="Perlin noise"/>
         <Settings>
            <setting type="QColor" id="color" value="#ffffff"/>
            <setting type="int" id="numoctaves" value="25"/>
            <setting type="double" id="persistence" value="0.7999999999999999"/>
            <setting type="double" id="randomizer" value="500"/>
            <setting type="double" id="zoom" value="40.36"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="2" name="Node 2">
         <pos x="22.44686850505258" y="-1168.230101981601"/>
         <generator name="Bricks"/>
         <Settings>
            <setting type="int" id="brickheight" value="38"/>
            <setting type="int" id="brickwidth" value="101"/>
            <setting type="QColor" id="color" value="#3f3f3f"/>
            <setting type="int" id="linewidth" value="10"/>
            <setting type="int" id="offsetx" value="0"/>
            <setting type="int" id="offsety" value="11"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="3" name="Node 3">
         <pos x="106.1252458451865" y="-227.7848615274045"/>
         <generator name="Fill"/>
         <Settings>
            <setting type="QColor" id="color" value="#7dff1b27"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="4" name="Node 4">
         <pos x="-789.1890911997839" y="149.1901681435517"/>
         <generator name="Blending"/>
         <Settings>
            <setting type="double" id="alpha" value="100"/>
            <setting type="QString" id="mode" value="Normal"/>
            <setting type="QString" id="order" value="Slot 2 on top of Slot 1"/>
         </Settings>
         <Sources>
            <source slot="0" source="1"/>
            <source slot="1" source="3"/>
         </Sources>
      </Node>
      <Node id="5" name="Node 5">
         <pos x="1094.132741125447" y="-386.6825894144326"/>
         <generator name="Sine transform"/>
         <Settings>
            <setting type="double" id="amplitudeone" value="0.10000000000000003"/>
            <setting type="double" id="amplitudetwo" value="0.30000000000000004"/>
            <setting type="double" id="angle" value="0"/>
            <setting type="double" id="frequencyone" value="40"/>
            <setting type="double" id="frequencytwo" value="20"/>
            <setting type="double" id="offsetone" value="0"/>
            <setting type="double" id="offsettwo" value="150.97"/>
         </Settings>
         <Sources>
            <source slot="0" source="2"/>
         </Sources>
      </Node>
      <Node id="6" name="Node 6">
         <pos x="-3.990333478359275" y="881.6970643678387"/>
         <generator name="Blending"/>
         <Settings>
            <setting type="double" id="alpha" value="100"/>
            <setting type="QString" id="mode" value="Normal"/>
            <setting type="QString" id="order" value="Slot 2 on top of Slot 1"/>
         </Settings>
         <Sources>
            <source slot="0" source="4"/>
            <source slot="1" source="8"/>
         </Sources>
      </Node>
      <Node id="7" name="Node 7">
         <pos x="1678.42611845819" y="579.113327153499"/>
         <generator name="Perlin noise"/>
         <Settings>
            <setting type="QColor" id="color" value="#d8d6e7"/>
            <setting type="int" id="numoctaves" value="10"/>
            <setting type="double" id="persistence" value="0.7999999999999999"/>
            <setting type="double" id="randomizer" value="500"/>
            <setting type="double" id="zoom" value="10"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="8" name="Node 8">
         <pos x="827.0347845547003" y="511.4229104483275"/>
         <generator name="Set channels"/>
         <Settings>
            <setting type="QString" id="channelAlpha" value="Second's alpha"/>
            <setting type="QString" id="channelBlue" value="First's blue"/>
            <setting type="QString" id="channelGreen" value="First's green"/>
            <setting type="QString" id="channelRed" value="First's red"/>
         </Settings>
         <Sources>
            <source slot="0" source="7"/>
            <source slot="1" source="5"/>
         </Sources>
      </Node>
      <Node id="9" name="Node 9">
         <pos x="-1562.074946154329" y="604.7787149901475"/>
         <generator name="Text"/>
         <Settings>
            <setting type="QColor" id="color" value="#ff0000"/>
            <setting type="QString" id="fontname" value="AnyStyle"/>
            <setting type="double" id="fontsize" value="33.97"/>
            <setting type="double" id="offsetleft" value="0"/>
            <setting type="double" id="offsettop" value="0"/>
            <setting type="double" id="rotation" value="0"/>
            <setting type="QString" id="text" value="Text"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="10" name="Node 10">
         <pos x="-586.8771110946963" y="1652.109149647044"/>
         <generator name="Glow"/>
         <Settings>
            <setting type="QColor" id="color" value="#ffff00"/>
            <setting type="double" id="cutoutx" value="95"/>
            <setting type="double" id="cutouty" value="95"/>
            <setting type="double" id="firstblurlevel" value="3"/>
            <setting type="" id="includesource" value=""/>
            <setting type="QString" id="mode" value="Multiply"/>
            <setting type="" id="ontop" value=""/>
            <setting type="double" id="secondblurlevel" value="3"/>
            <setting type="double" id="size" value="4"/>
         </Settings>
         <Sources>
            <source slot="0" source="9"/>
         </Sources>
      </Node>
      <Node id="13" name="Node 13">
         <pos x="947.1488595638025" y="1550.211099847647"/>
         <generator name="Blending"/>
         <Settings>
            <setting type="double" id="alpha" value="100"/>
            <setting type="QString" id="mode" value="Normal"/>
            <setting type="QString" id="order" value="Slot 2 on top of Slot 1"/>
         </Settings>
         <Sources>
            <source slot="0" source="6"/>
            <source slot="1" source="10"/>
         </Sources>
      </Node>
   </Nodes>
</TextureSet>
