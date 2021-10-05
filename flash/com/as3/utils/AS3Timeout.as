package com.as3.utils
{
	import flash.utils.setInterval;
	import flash.utils.clearInterval;
	import flash.events.EventDispatcher;
	import com.as3.utils.AS3Event;
	public class AS3Timeout extends EventDispatcher
	{
		private var timeoutSec:Number;
		private var timerId:Number=0;
		private var currentSec:Number=0;
		public function AS3Timeout(time:Number)
		{
			timeoutSec=time;
		}
		public function stop():void
		{
			currentSec=0;
			if (timerId!=0)
			{
				clearInterval(timerId);
				timerId=0;
			}
		}
		public function start():void
		{
			currentSec=0;
			if (timerId==0)
			{
				timerId=setInterval(function():void
				{
			    	if(++currentSec>=timeoutSec)
				    {
						currentSec=0;
			        	dispatchEvent(new AS3Event(AS3Event.TIMEOUT));
				    }
				},1000);
			}
		}
	}
}