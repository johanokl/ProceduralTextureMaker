<!DOCTYPE TextureSet>
<TextureSet>
   <Nodes>
      <Node name="Node 1" id="1">
         <pos y="-2141.961631203365" x="47.7894381779065"/>
         <generator name="Sine plasma"/>
         <Settings>
            <setting value="#ff0000" type="QColor" id="color"/>
            <setting value="41.71" type="double" id="xfrequency"/>
            <setting value="10" type="double" id="xoffset"/>
            <setting value="10" type="double" id="yfrequency"/>
            <setting value="10" type="double" id="yoffset"/>
         </Settings>
         <Sources>
            <source slot="0" source="9"/>
         </Sources>
      </Node>
      <Node name="Node 2" id="2">
         <pos y="-747.6099384752276" x="551.6851270822312"/>
         <generator name="Whirl"/>
         <Settings>
            <setting value="16.75" type="double" id="offsetleft"/>
            <setting value="0" type="double" id="offsettop"/>
            <setting value="42.11" type="double" id="radius"/>
            <setting value="404.31" type="double" id="strength"/>
         </Settings>
         <Sources/>
      </Node>
      <Node name="Node 5" id="5">
         <pos y="-692.4123633691182" x="-1862.277549965997"/>
         <generator name="Bricks"/>
         <Settings>
            <setting value="45" type="int" id="brickheight"/>
            <setting value="115" type="int" id="brickwidth"/>
            <setting value="#000000" type="QColor" id="color"/>
            <setting value="12" type="int" id="linewidth"/>
            <setting value="0" type="int" id="offsetx"/>
            <setting value="0" type="int" id="offsety"/>
         </Settings>
         <Sources>
            <source slot="0" source="1"/>
         </Sources>
      </Node>
      <Node name="Node 6" id="6">
         <pos y="1261.509135091383" x="956.0408663120647"/>
         <generator name="Blending"/>
         <Settings>
            <setting value="128" type="double" id="alpha"/>
            <setting value="Normal" type="QString" id="mode"/>
            <setting value="Slot 2 on top of Slot 1" type="QString" id="order"/>
         </Settings>
         <Sources>
            <source slot="0" source="2"/>
            <source slot="1" source="5"/>
         </Sources>
      </Node>
      <Node name="Node 8" id="8">
         <pos y="2302.76684815686" x="1850.230589300955"/>
         <generator name="Blending"/>
         <Settings>
            <setting value="128" type="double" id="alpha"/>
            <setting value="Normal" type="QString" id="mode"/>
            <setting value="Slot 2 on top of Slot 1" type="QString" id="order"/>
         </Settings>
         <Sources>
            <source slot="0" source="6"/>
         </Sources>
      </Node>
      <Node name="Node 9" id="9">
         <pos y="-3611.790902672552" x="309.3944084689608"/>
         <generator name="Fill"/>
         <Settings>
            <setting value="#000000" type="QColor" id="color"/>
         </Settings>
         <Sources/>
      </Node>
      <Node name="Node 10" id="10">
         <pos y="-974.2880858286283" x="2475.800728259418"/>
         <generator name="Lens"/>
         <Settings>
            <setting value="0" type="int" id="offsetleft"/>
            <setting value="0" type="int" id="offsettop"/>
            <setting value="71.77" type="double" id="size"/>
            <setting value="293.60000000000014" type="double" id="strength"/>
         </Settings>
         <Sources>
            <source slot="0" source="8"/>
         </Sources>
      </Node>
      <Node name="Node 12" id="12">
         <pos y="-2673.986580928769" x="2831.511297482321"/>
         <generator name="Whirl"/>
         <Settings>
            <setting value="0" type="double" id="offsetleft"/>
            <setting value="-11.96" type="double" id="offsettop"/>
            <setting value="55.5" type="double" id="radius"/>
            <setting value="313.4" type="double" id="strength"/>
         </Settings>
         <Sources>
            <source slot="0" source="10"/>
         </Sources>
      </Node>
      <Node name="Node 19" id="19">
         <pos y="-4478.782827204766" x="3908.754372847892"/>
         <generator name="Circle"/>
         <Settings>
            <setting value="#000000" type="QColor" id="color"/>
            <setting value="0" type="double" id="innerradius"/>
            <setting value="0" type="double" id="offsetleft"/>
            <setting value="-11.96" type="double" id="offsettop"/>
            <setting value="62.31" type="double" id="outerradius"/>
         </Settings>
         <Sources/>
      </Node>
      <Node name="Node 20" id="20">
         <pos y="-3464.03239486112" x="4249.636776996834"/>
         <generator name="Set channels"/>
         <Settings>
            <setting value="Second's alpha" type="QString" id="channelAlpha"/>
            <setting value="First's blue" type="QString" id="channelBlue"/>
            <setting value="First's green" type="QString" id="channelGreen"/>
            <setting value="First's red" type="QString" id="channelRed"/>
         </Settings>
         <Sources>
            <source slot="0" source="12"/>
            <source slot="1" source="19"/>
         </Sources>
      </Node>
      <Node name="Node 21" id="21">
         <pos y="-5726.367902737942" x="7122.137467449858"/>
         <generator name="Circle"/>
         <Settings>
            <setting value="#009c00" type="QColor" id="color"/>
            <setting value="0" type="double" id="innerradius"/>
            <setting value="0" type="double" id="offsetleft"/>
            <setting value="0" type="double" id="offsettop"/>
            <setting value="80.4" type="double" id="outerradius"/>
         </Settings>
         <Sources/>
      </Node>
      <Node name="Node 22" id="22">
         <pos y="-1345.802115668307" x="3804.487313128264"/>
         <generator name="Blending"/>
         <Settings>
            <setting value="128" type="double" id="alpha"/>
            <setting value="Normal" type="QString" id="mode"/>
            <setting value="Slot 2 on top of Slot 1" type="QString" id="order"/>
         </Settings>
         <Sources>
            <source slot="0" source="30"/>
            <source slot="1" source="33"/>
         </Sources>
      </Node>
      <Node name="Node 23" id="23">
         <pos y="-3318.110044461161" x="7152.256212905658"/>
         <generator name="Transform"/>
         <Settings>
            <setting value="#00000000" type="QColor" id="backgroundcolor"/>
            <setting value="1" type="int" id="firstXtiles"/>
            <setting value="1" type="int" id="firstYtiles"/>
            <setting value="16.35" type="double" id="offsetleft"/>
            <setting value="0" type="double" id="offsettop"/>
            <setting value="31.7" type="double" id="rotation"/>
            <setting value="1" type="int" id="secondXtiles"/>
            <setting value="1" type="int" id="secondYtiles"/>
            <setting value="23.9" type="double" id="xscale"/>
            <setting value="66.67" type="double" id="yscale"/>
         </Settings>
         <Sources>
            <source slot="0" source="21"/>
         </Sources>
      </Node>
      <Node name="Node 24" id="24">
         <pos y="-2339.724688409451" x="6392.799964580176"/>
         <generator name="Blending"/>
         <Settings>
            <setting value="128" type="double" id="alpha"/>
            <setting value="Normal" type="QString" id="mode"/>
            <setting value="Slot 2 on top of Slot 1" type="QString" id="order"/>
         </Settings>
         <Sources>
            <source slot="0" source="23"/>
            <source slot="1" source="26"/>
         </Sources>
      </Node>
      <Node name="Node 23" id="26">
         <pos y="-3970.567237674133" x="5822.27294053576"/>
         <generator name="Transform"/>
         <Settings>
            <setting value="#00000000" type="QColor" id="backgroundcolor"/>
            <setting value="1" type="int" id="firstXtiles"/>
            <setting value="1" type="int" id="firstYtiles"/>
            <setting value="-11.32" type="double" id="offsetleft"/>
            <setting value="0" type="double" id="offsettop"/>
            <setting value="142.64" type="double" id="rotation"/>
            <setting value="1" type="int" id="secondXtiles"/>
            <setting value="1" type="int" id="secondYtiles"/>
            <setting value="23.9" type="double" id="xscale"/>
            <setting value="66.67" type="double" id="yscale"/>
         </Settings>
         <Sources>
            <source slot="0" source="21"/>
         </Sources>
      </Node>
      <Node name="Node 27" id="27">
         <pos y="276.9620699502432" x="5678.341116018087"/>
         <generator name="Blending"/>
         <Settings>
            <setting value="100" type="double" id="alpha"/>
            <setting value="Normal" type="QString" id="mode"/>
            <setting value="Slot 1 on top of Slot 2" type="QString" id="order"/>
         </Settings>
         <Sources>
            <source slot="0" source="29"/>
            <source slot="1" source="28"/>
         </Sources>
      </Node>
      <Node name="Node 28" id="28">
         <pos y="-788.6858242130257" x="7120.392828901847"/>
         <generator name="Square"/>
         <Settings>
            <setting value="#009c00" type="QColor" id="color"/>
            <setting value="0" type="double" id="cutoutheight"/>
            <setting value="0" type="double" id="cutoutwidth"/>
            <setting value="51.81" type="double" id="height"/>
            <setting value="2.59" type="double" id="offsetleft"/>
            <setting value="4.66" type="double" id="offsettop"/>
            <setting value="0" type="double" id="rotation"/>
            <setting value="9.33" type="double" id="width"/>
         </Settings>
         <Sources/>
      </Node>
      <Node name="Node 29" id="29">
         <pos y="-1134.082169320552" x="5661.753730806418"/>
         <generator name="Transform"/>
         <Settings>
            <setting value="#00000000" type="QColor" id="backgroundcolor"/>
            <setting value="1" type="int" id="firstXtiles"/>
            <setting value="1" type="int" id="firstYtiles"/>
            <setting value="0" type="double" id="offsetleft"/>
            <setting value="18.87" type="double" id="offsettop"/>
            <setting value="0" type="double" id="rotation"/>
            <setting value="1" type="int" id="secondXtiles"/>
            <setting value="1" type="int" id="secondYtiles"/>
            <setting value="80" type="double" id="xscale"/>
            <setting value="80" type="double" id="yscale"/>
         </Settings>
         <Sources>
            <source slot="0" source="24"/>
         </Sources>
      </Node>
      <Node name="Node 30" id="30">
         <pos y="-389.570210835304" x="4822.01349856143"/>
         <generator name="Transform"/>
         <Settings>
            <setting value="#00000000" type="QColor" id="backgroundcolor"/>
            <setting value="1" type="int" id="firstXtiles"/>
            <setting value="1" type="int" id="firstYtiles"/>
            <setting value="0" type="double" id="offsetleft"/>
            <setting value="26.42" type="double" id="offsettop"/>
            <setting value="0" type="double" id="rotation"/>
            <setting value="1" type="int" id="secondXtiles"/>
            <setting value="1" type="int" id="secondYtiles"/>
            <setting value="60" type="double" id="xscale"/>
            <setting value="60" type="double" id="yscale"/>
         </Settings>
         <Sources>
            <source slot="0" source="27"/>
         </Sources>
      </Node>
      <Node name="Node 31" id="31">
         <pos y="1285.217621587164" x="2934.76225495929"/>
         <generator name="Gradient"/>
         <Settings>
            <setting value="#793c00" type="QColor" id="endcolor"/>
            <setting value="0" type="double" id="endposx"/>
            <setting value="34.13" type="double" id="endposy"/>
            <setting value="Linear Gradient" type="QString" id="gradient"/>
            <setting value="#00aaff" type="QColor" id="middlecolor"/>
            <setting value="93.41" type="double" id="middleposition"/>
            <setting value="50" type="double" id="radius"/>
            <setting value="Pad Spread" type="QString" id="spread"/>
            <setting value="#640055ff" type="QColor" id="startcolor"/>
            <setting value="-10" type="double" id="startposx"/>
            <setting value="-46.11" type="double" id="startposy"/>
         </Settings>
         <Sources/>
      </Node>
      <Node name="Final" id="32">
         <pos y="125.164104877592" x="3558.074592295776"/>
         <generator name="Blending"/>
         <Settings>
            <setting value="100" type="double" id="alpha"/>
            <setting value="Normal" type="QString" id="mode"/>
            <setting value="Slot 2 on top of Slot 1" type="QString" id="order"/>
         </Settings>
         <Sources>
            <source slot="0" source="31"/>
            <source slot="1" source="22"/>
         </Sources>
      </Node>
      <Node name="Node 33" id="33">
         <pos y="-2276.821454437443" x="4986.49869869189"/>
         <generator name="Sine transform"/>
         <Settings>
            <setting value="10" type="double" id="amplitudeone"/>
            <setting value="2" type="double" id="amplitudetwo"/>
            <setting value="45" type="double" id="angle"/>
            <setting value="0.2" type="double" id="frequencyone"/>
            <setting value="8" type="double" id="frequencytwo"/>
            <setting value="0" type="double" id="offsetone"/>
            <setting value="0" type="double" id="offsettwo"/>
         </Settings>
         <Sources>
            <source slot="0" source="20"/>
         </Sources>
      </Node>
   </Nodes>
</TextureSet>
