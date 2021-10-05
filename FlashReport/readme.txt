Multi-series Column 3D Line Dual Y Combination Chart 
MSColumn3DLineDY.swf

chart:
	PYAxisName		左名称(主)
	SYAxisName		右名称(副)
	xAxisName			底部名称

	animation			是否动画
	palette				1-5,背景颜色
	showLabels		是否显示底部文字
	labelDisplay 	WRAP, STAGGER, ROTATE or NONE
	rotateLabels	底部文字是否垂直
	slantLabels		底部文字垂直时，是否倾斜
	labelStep			底部文字每隔几个才显示
	staggerLines
	connectNullData	没有数据是否连线
	showValues		是否显示数据的值
	rotateValues	竖显示数据的值
	showYAxisValues	是否显示两边的值
	showLimits		限制两边的值显示
	showDivLineValues 是否两边每中间间隔的值
	yAxisValuesStep	两边值显示间隔
	adjustDiv  Boolean  0/1  FusionCharts automatically tries to adjust divisional lines and limit values based on the data provided. However, if you want to set your explicit lower and upper limit values and number of divisional lines, first set this attribute to false. That would disable automatic adjustment of divisional lines.  
showSecondaryLimits  Boolean  0/1  Whether to show secondary axis chart limit values?  
showDivLineSecondaryValue  Boolean  0/1  Whether to show div line values for the secondary y-axis?  
clickURL  String   The entire chart can now act as a hotspot. Use this URL to define the hotspot link for the chart. The link can be specified in FusionCharts Link Format.  
maxColWidth  Number  In Pixels  Maximum allowed column width  
use3DLighting  Boolean  0/1  Whether to use advanced gradients and shadow effects to create better looking 3D charts?  
defaultAnimation  Boolean  0/1  By default, each chart animates some of its elements. If you wish to switch off the default animation patterns, you can set this attribute to 0. It can be particularly useful when you want to define your own animation patterns using STYLE feature.  
showShadow  Boolean  0/1  Whether to show shadows for data plot?  
PYAxisMaxValue  Number   This attribute helps you explicitly set the upper limit of the primary y-axis. If you don't specify this value, it is automatically calculated by FusionCharts based on the data provided by you.  
PYAxisMinValue  Number   This attribute helps you explicitly set the lower limit of the primary y-axis. If you don't specify this value, it is automatically calculated by FusionCharts based on the data provided by you.  
SYAxisMinValue  Number   This attribute helps you explicitly set the lower limit of the secondary y-axis. If you don't specify this value, it is automatically calculated by FusionCharts based on the data provided by you.  
SYAxisMaxValue  Number   This attribute helps you explicitly set the upper limit of the secondary y-axis. If you don't specify this value, it is automatically calculated by FusionCharts based on the data provided by you.  
setAdaptiveYMin  Boolean  0/1  This attribute lets you set whether the y-axis lower limit would be 0 (in case of all positive values on chart) or should the y-axis lower limit adapt itself to a different figure based on values provided to the chart.  
setAdaptiveSYMin  Boolean  0/1  This attribute lets you set whether the secondary y-axis lower limit would be 0 (in case of all positive values on chart) or should the y-axis lower limit adapt itself to a different figure based on values provided to the chart.  
rotateYAxisName  Boolean  0/1  If you do not wish to rotate y-axis name, set this as 0. It specifically comes to use when you've special characters (UTF8) in your y-axis name that do not show up in rotated mode.  
PYAxisNameWidth  Number  (In Pixels)  If you opt to not rotate y-axis name, you can choose a maximum width that will be applied to primary y-axis name.  
SYAxisNameWidth  Number  (In Pixels)  If you opt to not rotate y-axis name, you can choose a maximum width that will be applied to secondary y-axis name.  


<chart caption='标题' subcaption='副标题' showValues='0' PYAxisName='Sales' SYAxisName='Total Downloads'>
   <categories>
      <category label='Jan'/>
      <category label='Feb'/>
      <category label='Mar'/>
   </categories>

   <dataset seriesName='Product A Sales'>
      <set value='230' />
      <set value='245' />
      <set value='250' />
   </dataset>
   <dataset seriesName='Product B Sales'>
      <set value='130' />
      <set value='145' />
      <set value='50' />
   </dataset>
   <dataset seriesName='Total Downloads' parentYAxis='S'>
      <set value='13000' />
      <set value='14500' />
      <set value='5000' />
   </dataset>

   <trendlines>
      <line startValue='300' color='91C728' displayValue='Target' showOnTop='1'/>
   </trendlines>

</chart>
