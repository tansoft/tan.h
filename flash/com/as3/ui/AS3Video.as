﻿package com.as3.ui{	import flash.display.Sprite;	import flash.display.MovieClip;	import flash.events.MouseEvent;	import flash.events.Event;	import flash.utils.setTimeout;	import flash.geom.Rectangle;	import com.as3.ui.*;	import com.as3.utils.*;	import flash.media.Video;	import flash.net.NetStream;	public class AS3Video extends Sprite	{		protected var _video:Video;		protected var _hit_spr:Sprite;		protected var _scale:Number=0;		protected var _rscale:Number=0;		protected var _rotation:Number=0;		protected var _w:int=0;		protected var _h:int=0;		public function AS3Video()		{			super();			_video=new Video();			_video.smoothing=true;			addChild(_video);			_hit_spr=new Sprite();			AS3Utils.drawRect(_hit_spr, 0, 0, 1, 1, 0xffffff, 0);			_hit_spr.doubleClickEnabled=true;			addChild(_hit_spr);		}		public function setScale(w:Number,h:Number):void		{			if (h!=0) _scale=w/h;			else _scale=0;			if (_scale!=0) _rscale=1/_scale;			else _rscale=0;			resize(_w,_h,null);		}		public function setScalebyScale(scale:Number):void		{			_scale=scale;			if (_scale!=0) _rscale=1/_scale;			else _rscale=0;			resize(_w,_h,null);		}		public function setRotation(rotation:Number):void		{			var r:Number=rotation;			if (r>=360) r%=360;			this.rotation=r;			_rotation=Math.round(r/90);			resize(_w,_h,null);		}		public function addRotation(rotation:Number):void		{			var r:Number=_rotation*90+rotation;			if (r>=360) r%=360;			this.rotation=r;			_rotation=Math.round(r/90);			resize(_w,_h,null);		}		public function attachNetStream(ns:NetStream)		{			_video.attachNetStream(ns);		}		override public function addEventListener(type:String,fn:Function,b:Boolean=false,p:int=0,w:Boolean=false):void		{			_hit_spr.addEventListener(type,fn,b,p,w);		}		public function resize(w:int,h:int,obj:Object):void		{			if (obj!=null) {				if (!(obj is Number)) {					if (obj["cx"]!=undefined) w=w+obj["cx"];					if (obj["cy"]!=undefined) h=h+obj["cy"];				}			}			_w=w;			_h=h;			var t:int=_rotation%2;//r=0|2 t=0 r=1|3 t=1			var r:int=_rotation-1-t;//_r=0|2 r=_r-1 _r=1|3 r=_r-2			var sx:int=0;			var sy:int=0;			if (_scale!=0 && h!=0)			{				if (t==0) {					if (w/h>_scale) //the h higher						sx=(w-h*_scale)/2;					else //the w weighter						sy=(h-w/_scale)/2;				} else {					if (_rscale!=0 && h!=0)					{						if (w/h>_rscale) //the h higher							sx=(w-h*_rscale)/2;						else //the w weighter							sy=(h-w/_rscale)/2;					}				}			}			if (t==0) {				_video.x=-w/2;				_video.y=-h/2;				_video.width=w-sx*2;				_video.height=h-sy*2;				_hit_spr.x=-w/2-sx;				_hit_spr.y=-h/2-sy;				_hit_spr.width=w;				_hit_spr.height=h;				x=-_video.x-r*sx;				y=-_video.y-r*sy;			} else {				_video.x=-h/2;				_video.y=-w/2;				_video.width=h-sy*2;				_video.height=w-sx*2;				_hit_spr.x=-h/2-sy;				_hit_spr.y=-w/2-sx;				_hit_spr.width=h;				_hit_spr.height=w;				x=-_video.y+r*sx;				y=-_video.x-r*sy;			}		}	}}