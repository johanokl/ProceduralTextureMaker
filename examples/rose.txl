<!DOCTYPE TextureSet>
<TextureSet>
   <Nodes>
      <Node id="1" name="Node 1">
         <pos y="-1445.7" x="1262.01"/>
         <generator name="Sine plasma"/>
         <Settings>
            <setting id="color" type="QColor" value="#ff0000"/>
            <setting id="xfrequency" type="double" value="41.71"/>
            <setting id="xoffset" type="double" value="10"/>
            <setting id="yfrequency" type="double" value="10"/>
            <setting id="yoffset" type="double" value="10"/>
         </Settings>
         <Sources>
            <source slot="0" source="9"/>
         </Sources>
      </Node>
      <Node id="5" name="Node 5">
         <pos y="-855.645" x="1260.38"/>
         <generator name="Bricks"/>
         <Settings>
            <setting id="brickheight" type="int" value="45"/>
            <setting id="brickwidth" type="int" value="115"/>
            <setting id="color" type="QColor" value="#000000"/>
            <setting id="linewidth" type="int" value="12"/>
            <setting id="offsetx" type="int" value="0"/>
            <setting id="offsety" type="int" value="0"/>
         </Settings>
         <Sources>
            <source slot="0" source="1"/>
         </Sources>
      </Node>
      <Node id="9" name="Node 9">
         <pos y="-1438.45" x="1748.23"/>
         <generator name="Fill"/>
         <Settings>
            <setting id="color" type="QColor" value="#000000"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="10" name="Node 10">
         <pos y="-836.258" x="1771.49"/>
         <generator name="Lens"/>
         <Settings>
            <setting id="offsetleft" type="int" value="0"/>
            <setting id="offsettop" type="int" value="0"/>
            <setting id="size" type="double" value="71.77"/>
            <setting id="strength" type="double" value="293.60000000000014"/>
         </Settings>
         <Sources>
            <source slot="0" source="5"/>
         </Sources>
      </Node>
      <Node id="12" name="Node 12">
         <pos y="-1417.1" x="2234.78"/>
         <generator name="Whirl"/>
         <Settings>
            <setting id="offsetleft" type="double" value="0"/>
            <setting id="offsettop" type="double" value="-11.96"/>
            <setting id="radius" type="double" value="55.5"/>
            <setting id="strength" type="double" value="313.4"/>
         </Settings>
         <Sources>
            <source slot="0" source="10"/>
         </Sources>
      </Node>
      <Node id="19" name="Node 19">
         <pos y="-2110.73" x="1892.55"/>
         <generator name="Circle"/>
         <Settings>
            <setting id="color" type="QColor" value="#000000"/>
            <setting id="innerradius" type="double" value="0"/>
            <setting id="offsetleft" type="double" value="0"/>
            <setting id="offsettop" type="double" value="-11.96"/>
            <setting id="outerradius" type="double" value="62.31"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="20" name="Node 20">
         <pos y="-1908.16" x="2319.44"/>
         <generator name="Set channels"/>
         <Settings>
            <setting id="channelAlpha" type="QString" value="Second's alpha"/>
            <setting id="channelBlue" type="QString" value="First's blue"/>
            <setting id="channelGreen" type="QString" value="First's green"/>
            <setting id="channelRed" type="QString" value="First's red"/>
         </Settings>
         <Sources>
            <source slot="0" source="12"/>
            <source slot="1" source="19"/>
         </Sources>
      </Node>
      <Node id="21" name="Node 21">
         <pos y="-1546.51" x="5645.49"/>
         <generator name="Circle"/>
         <Settings>
            <setting id="color" type="QColor" value="#009c00"/>
            <setting id="innerradius" type="double" value="0"/>
            <setting id="offsetleft" type="double" value="0"/>
            <setting id="offsettop" type="double" value="0"/>
            <setting id="outerradius" type="double" value="80.4"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="22" name="Node 22">
         <pos y="-809.807" x="3273.11"/>
         <generator name="Blending"/>
         <Settings>
            <setting id="alpha" type="double" value="128"/>
            <setting id="mode" type="QString" value="Normal"/>
            <setting id="order" type="QString" value="Slot 2 on top of Slot 1"/>
         </Settings>
         <Sources>
            <source slot="0" source="30"/>
            <source slot="1" source="33"/>
         </Sources>
      </Node>
      <Node id="23" name="Node 23">
         <pos y="-763.285" x="5643.66"/>
         <generator name="Transform"/>
         <Settings>
            <setting id="backgroundcolor" type="QColor" value="#00000000"/>
            <setting id="firstXtiles" type="int" value="1"/>
            <setting id="firstYtiles" type="int" value="1"/>
            <setting id="offsetleft" type="double" value="16.35"/>
            <setting id="offsettop" type="double" value="0"/>
            <setting id="rotation" type="double" value="31.7"/>
            <setting id="secondXtiles" type="int" value="1"/>
            <setting id="secondYtiles" type="int" value="1"/>
            <setting id="xscale" type="double" value="23.9"/>
            <setting id="yscale" type="double" value="66.67"/>
         </Settings>
         <Sources>
            <source slot="0" source="21"/>
         </Sources>
      </Node>
      <Node id="24" name="Node 24">
         <pos y="-930.607" x="5098.36"/>
         <generator name="Blending"/>
         <Settings>
            <setting id="alpha" type="double" value="128"/>
            <setting id="mode" type="QString" value="Normal"/>
            <setting id="order" type="QString" value="Slot 2 on top of Slot 1"/>
         </Settings>
         <Sources>
            <source slot="0" source="23"/>
            <source slot="1" source="26"/>
         </Sources>
      </Node>
      <Node id="26" name="Node 23">
         <pos y="-1569.45" x="4987.24"/>
         <generator name="Transform"/>
         <Settings>
            <setting id="backgroundcolor" type="QColor" value="#00000000"/>
            <setting id="firstXtiles" type="int" value="1"/>
            <setting id="firstYtiles" type="int" value="1"/>
            <setting id="offsetleft" type="double" value="-11.32"/>
            <setting id="offsettop" type="double" value="0"/>
            <setting id="rotation" type="double" value="142.64"/>
            <setting id="secondXtiles" type="int" value="1"/>
            <setting id="secondYtiles" type="int" value="1"/>
            <setting id="xscale" type="double" value="23.9"/>
            <setting id="yscale" type="double" value="66.67"/>
         </Settings>
         <Sources>
            <source slot="0" source="21"/>
         </Sources>
      </Node>
      <Node id="27" name="Node 27">
         <pos y="-798.982" x="3958.45"/>
         <generator name="Blending"/>
         <Settings>
            <setting id="alpha" type="double" value="100"/>
            <setting id="mode" type="QString" value="Normal"/>
            <setting id="order" type="QString" value="Slot 1 on top of Slot 2"/>
         </Settings>
         <Sources>
            <source slot="0" source="29"/>
            <source slot="1" source="28"/>
         </Sources>
      </Node>
      <Node id="28" name="Node 28">
         <pos y="-1301.39" x="4455.22"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#009c00"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="51.81"/>
            <setting id="offsetleft" type="double" value="2.59"/>
            <setting id="offsettop" type="double" value="4.66"/>
            <setting id="rotation" type="double" value="0"/>
            <setting id="width" type="double" value="9.33"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="29" name="Node 29">
         <pos y="-597.505" x="4565.87"/>
         <generator name="Transform"/>
         <Settings>
            <setting id="backgroundcolor" type="QColor" value="#00000000"/>
            <setting id="firstXtiles" type="int" value="1"/>
            <setting id="firstYtiles" type="int" value="1"/>
            <setting id="offsetleft" type="double" value="0"/>
            <setting id="offsettop" type="double" value="18.87"/>
            <setting id="rotation" type="double" value="0"/>
            <setting id="secondXtiles" type="int" value="1"/>
            <setting id="secondYtiles" type="int" value="1"/>
            <setting id="xscale" type="double" value="80"/>
            <setting id="yscale" type="double" value="80"/>
         </Settings>
         <Sources>
            <source slot="0" source="24"/>
         </Sources>
      </Node>
      <Node id="30" name="Node 30">
         <pos y="-1393.3" x="3571.37"/>
         <generator name="Transform"/>
         <Settings>
            <setting id="backgroundcolor" type="QColor" value="#00000000"/>
            <setting id="firstXtiles" type="int" value="1"/>
            <setting id="firstYtiles" type="int" value="1"/>
            <setting id="offsetleft" type="double" value="0"/>
            <setting id="offsettop" type="double" value="26.42"/>
            <setting id="rotation" type="double" value="0"/>
            <setting id="secondXtiles" type="int" value="1"/>
            <setting id="secondYtiles" type="int" value="1"/>
            <setting id="xscale" type="double" value="60"/>
            <setting id="yscale" type="double" value="60"/>
         </Settings>
         <Sources>
            <source slot="0" source="27"/>
         </Sources>
      </Node>
      <Node id="31" name="Node 31">
         <pos y="-761.597" x="2408.9"/>
         <generator name="Gradient"/>
         <Settings>
            <setting id="endcolor" type="QColor" value="#793c00"/>
            <setting id="endposx" type="double" value="0"/>
            <setting id="endposy" type="double" value="34.13"/>
            <setting id="gradient" type="QString" value="Linear Gradient"/>
            <setting id="middlecolor" type="QColor" value="#00aaff"/>
            <setting id="middleposition" type="double" value="93.41"/>
            <setting id="radius" type="double" value="50"/>
            <setting id="spread" type="QString" value="Pad Spread"/>
            <setting id="startcolor" type="QColor" value="#640055ff"/>
            <setting id="startposx" type="double" value="-10"/>
            <setting id="startposy" type="double" value="-46.11"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="32" name="Final">
         <pos y="-252.352" x="2916.06"/>
         <generator name="Blending"/>
         <Settings>
            <setting id="alpha" type="double" value="100"/>
            <setting id="mode" type="QString" value="Normal"/>
            <setting id="order" type="QString" value="Slot 2 on top of Slot 1"/>
         </Settings>
         <Sources>
            <source slot="0" source="31"/>
            <source slot="1" source="22"/>
         </Sources>
      </Node>
      <Node id="33" name="Node 33">
         <pos y="-1343.42" x="2882.19"/>
         <generator name="Sine transform"/>
         <Settings>
            <setting id="amplitudeone" type="double" value="10"/>
            <setting id="amplitudetwo" type="double" value="2"/>
            <setting id="angle" type="double" value="45"/>
            <setting id="frequencyone" type="double" value="0.2"/>
            <setting id="frequencytwo" type="double" value="8"/>
            <setting id="offsetone" type="double" value="0"/>
            <setting id="offsettwo" type="double" value="0"/>
         </Settings>
         <Sources>
            <source slot="0" source="20"/>
         </Sources>
      </Node>
   </Nodes>
</TextureSet>
