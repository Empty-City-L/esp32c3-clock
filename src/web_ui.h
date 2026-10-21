/**
 * web_ui.h — 内置配网页面（全部内联，无外部 CDN）
 *
 * 页面风格与 160×128 时钟一致：深海军蓝 + 青色强调 + 玻璃卡片。
 * 所有资源都在这个字符串里，断网也能正常打开。
 */
#pragma once

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="zh-CN">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#060a12">
<meta name="color-scheme" content="dark">
<title>ESP32-C3 桌面时钟 · 配网与设置</title>
<link rel="icon" href="data:,">
<style>
:root{
  --bg:#060a12;--panel:rgba(15,24,40,.8);--panel-2:rgba(8,14,24,.92);
  --line:rgba(148,163,184,.14);--line-2:#20324d;
  --text:#eef5ff;--soft:#b8c8de;--muted:#93a4bd;--dim:#6b7c96;
  --accent:#5eead4;--accent-2:#60a5fa;--green:#34d399;--amber:#fbbf24;--red:#fb7185;
  --radius:20px;--shadow:0 24px 60px rgba(0,0,0,.42);
}
*{box-sizing:border-box}
[hidden]{display:none!important}
html,body{margin:0;padding:0}
body{
  min-height:100vh;overflow-x:hidden;background:var(--bg);color:var(--text);
  font-family:-apple-system,BlinkMacSystemFont,"Segoe UI","Microsoft YaHei","PingFang SC",sans-serif;
  -webkit-font-smoothing:antialiased;text-rendering:optimizeLegibility;
}
button,input,select{font:inherit}
button{cursor:pointer}
.aurora{position:fixed;inset:0;z-index:-2;pointer-events:none;background:
  radial-gradient(900px 520px at 86% -8%,rgba(94,234,212,.16),transparent 62%),
  radial-gradient(760px 500px at 2% 8%,rgba(96,165,250,.14),transparent 60%),
  radial-gradient(900px 640px at 50% 118%,rgba(94,234,212,.07),transparent 62%),
  linear-gradient(180deg,#070c16 0%,#060a12 46%,#05080f 100%);}
.aurora:after{content:"";position:absolute;inset:0;opacity:.55;background-image:
  linear-gradient(rgba(94,234,212,.055) 1px,transparent 1px),
  linear-gradient(90deg,rgba(94,234,212,.055) 1px,transparent 1px);
  background-size:56px 56px;
  -webkit-mask-image:radial-gradient(ellipse at 50% 0%,#000 0%,transparent 72%);
  mask-image:radial-gradient(ellipse at 50% 0%,#000 0%,transparent 72%);}
.wrap{max-width:1060px;margin:0 auto;padding:32px 20px 120px}
.top{display:flex;align-items:center;justify-content:space-between;gap:18px;flex-wrap:wrap;margin-bottom:22px}
.brand{display:flex;align-items:center;gap:14px;min-width:0}
.logo{width:54px;height:54px;flex:0 0 auto;border-radius:17px;display:grid;place-items:center;color:var(--accent);
  background:linear-gradient(145deg,rgba(94,234,212,.16),rgba(96,165,250,.1));
  border:1px solid rgba(94,234,212,.28);
  box-shadow:0 12px 34px rgba(94,234,212,.14),inset 0 1px 0 rgba(255,255,255,.08)}
.logo svg{width:34px;height:34px;display:block}
h1{margin:0;font-size:clamp(20px,3.2vw,26px);line-height:1.18;letter-spacing:.01em}
h1 span{color:var(--accent)}
.brand p{margin:5px 0 0;color:var(--dim);font-size:12px;letter-spacing:.04em}
.status{display:flex;align-items:center;gap:8px;flex-wrap:wrap}
.pill{display:inline-flex;align-items:center;gap:8px;min-height:36px;padding:0 13px;border-radius:999px;
  background:rgba(14,24,40,.78);border:1px solid var(--line);font-size:12px;color:var(--soft);white-space:nowrap}
.dot{width:7px;height:7px;border-radius:50%;background:var(--dim);flex:0 0 auto;
  box-shadow:0 0 0 4px rgba(107,124,150,.12);transition:.2s}
.dot.online{background:var(--green);box-shadow:0 0 0 4px rgba(52,211,153,.14),0 0 14px rgba(52,211,153,.65)}
.dot.ap{background:var(--amber);box-shadow:0 0 0 4px rgba(251,191,36,.14),0 0 14px rgba(251,191,36,.5)}
.dot.off{background:var(--red);box-shadow:0 0 0 4px rgba(251,113,133,.12)}
.ip{font-family:ui-monospace,SFMono-Regular,Consolas,"Liberation Mono",monospace;color:var(--accent)}
.banner{display:flex;gap:10px;align-items:flex-start;margin:0 0 18px;padding:13px 16px;border-radius:15px;
  background:linear-gradient(135deg,rgba(251,191,36,.1),rgba(251,191,36,.03));
  border:1px solid rgba(251,191,36,.28);color:#f7d894;font-size:12.5px;line-height:1.65}
.banner strong{color:var(--amber);white-space:nowrap}
.banner code{font-family:ui-monospace,Consolas,monospace;color:#ffe9b3}
.overview{display:grid;grid-template-columns:1.25fr 1fr;gap:26px;align-items:center;
  padding:26px;margin-bottom:18px}
.eyebrow{font:700 10px/1 ui-monospace,SFMono-Regular,Consolas,monospace;letter-spacing:.22em;color:var(--dim);margin-bottom:10px}
.device-clock{display:flex;align-items:baseline;gap:10px;font:700 clamp(48px,8.2vw,78px)/.94 ui-monospace,SFMono-Regular,Consolas,monospace;
  letter-spacing:-.055em;color:var(--text)}
.device-clock #deviceSec{font-size:.36em;letter-spacing:0;color:var(--accent);transform:translateY(-.12em)}
.device-date{margin-top:11px;color:var(--muted);font-size:12.5px;letter-spacing:.03em}
.stats{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.stat{padding:12px 14px;border-radius:14px;background:rgba(6,12,22,.62);border:1px solid var(--line);min-width:0}
.stat span{display:block;font-size:10.5px;letter-spacing:.1em;color:var(--dim);margin-bottom:6px;text-transform:uppercase}
.stat strong{display:block;font-size:13px;font-weight:600;color:var(--soft);white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.grid{display:grid;grid-template-columns:1fr 1fr;gap:18px}
.panel{position:relative;overflow:hidden;background:linear-gradient(168deg,rgba(17,27,45,.88),rgba(9,15,26,.94));
  border:1px solid var(--line);border-radius:var(--radius);padding:22px;box-shadow:var(--shadow);
  backdrop-filter:blur(18px);-webkit-backdrop-filter:blur(18px)}
.panel:before{content:"";position:absolute;left:16%;right:16%;top:0;height:1px;
  background:linear-gradient(90deg,transparent,rgba(94,234,212,.5),transparent);opacity:.65}
.overview:before{left:8%;right:8%}
.panel-head{display:flex;align-items:flex-start;gap:12px;margin-bottom:18px}
.panel-head .ico{width:38px;height:38px;flex:0 0 auto;border-radius:12px;display:grid;place-items:center;color:var(--accent);
  background:rgba(94,234,212,.08);border:1px solid rgba(94,234,212,.18)}
.panel-head .ico svg{width:20px;height:20px;display:block}
.panel-head h2{font-size:15.5px;margin:0 0 4px;letter-spacing:.02em}
.panel-head p{font-size:11.5px;line-height:1.55;margin:0;color:var(--dim)}
.step{margin-left:auto;flex:0 0 auto;padding:5px 7px;border-radius:7px;color:rgba(94,234,212,.8);
  border:1px solid rgba(94,234,212,.2);background:rgba(94,234,212,.05);
  font:700 10px/1 ui-monospace,SFMono-Regular,Consolas,monospace;letter-spacing:.1em}
.field{display:flex;flex-direction:column;gap:7px;margin-bottom:14px;min-width:0}
.field:last-child{margin-bottom:0}
label{font-size:12px;color:var(--soft);letter-spacing:.02em}
.label-row{display:flex;align-items:center;justify-content:space-between;gap:10px}
.value{font:600 12px ui-monospace,Consolas,monospace;color:var(--accent)}
input,select{width:100%;height:44px;border-radius:12px;border:1px solid var(--line-2);
  background:#080f1c;color:var(--text);padding:0 13px;font-size:14px;outline:none;
  transition:border-color .18s,box-shadow .18s,background .18s}
input::placeholder{color:#566880}
input:focus,select:focus{border-color:rgba(94,234,212,.68);box-shadow:0 0 0 4px rgba(94,234,212,.1);background:#0a1424}
select{appearance:none;-webkit-appearance:none;padding-right:38px;cursor:pointer;
  background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='16' height='16' viewBox='0 0 24 24' fill='none' stroke='%237f93af' stroke-width='2' stroke-linecap='round' stroke-linejoin='round'%3E%3Cpath d='m6 9 6 6 6-6'/%3E%3C/svg%3E");
  background-repeat:no-repeat;background-position:right 13px center}
.two{display:grid;grid-template-columns:1fr 1fr;gap:12px}
.three{display:grid;grid-template-columns:1.1fr 1.3fr 1fr;gap:12px}
.photo-layout{display:grid;grid-template-columns:320px 1fr;gap:18px;align-items:start}
.photo-canvas-wrap{padding:8px;border-radius:14px;background:#04080f;border:1px solid var(--line-2)}
#photoCanvas{display:block;width:100%;height:auto;border-radius:9px;background:#02060c}
.photo-drop{padding:18px 14px;border-radius:13px;border:1px dashed #3a5574;background:rgba(8,15,25,.65);
  text-align:center;cursor:pointer;transition:border-color .2s,background .2s}
.photo-drop:hover,.photo-drop.over{border-color:var(--accent);background:rgba(94,234,212,.06)}
.photo-drop b{display:block;font-size:13px;color:var(--soft);margin-bottom:5px}
.photo-drop span{font-size:11.5px;color:var(--dim)}
.photo-tools{display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-top:12px}
.photo-actions{display:flex;gap:10px;margin-top:12px;flex-wrap:wrap}
.photo-actions .btn{min-height:40px;padding:9px 14px;font-size:12px}
.photo-gallery{display:grid;grid-template-columns:repeat(auto-fill,minmax(150px,1fr));gap:12px;margin-top:14px}
.photo-thumb{position:relative;min-width:0}
.photo-thumb canvas{display:block;width:100%;aspect-ratio:5/4;object-fit:cover;border-radius:9px;border:1px solid #2c405a;background:#04080f}
.photo-thumb button{position:absolute;top:4px;right:4px;width:22px;height:22px;border-radius:50%;border:1px solid rgba(255,255,255,.25);
  background:rgba(4,8,14,.78);color:#e6eefb;font-size:14px;line-height:1;cursor:pointer}
.photo-thumb small{display:block;margin-top:4px;font-size:10px;color:var(--dim);text-align:center}
.photo-empty{padding:18px;text-align:center;color:var(--dim);font-size:12px;border:1px solid var(--line);border-radius:10px}
.input-row{display:flex;gap:8px;align-items:center}
.input-row input{flex:1;min-width:0}
.hint{margin:0;font-size:11px;line-height:1.55;color:var(--dim)}
.hint code{font-family:ui-monospace,Consolas,monospace;color:var(--soft);background:rgba(148,163,184,.1);
  border-radius:5px;padding:1px 5px}
.hint.warn{color:#a4b3c8}
.btn{display:inline-flex;align-items:center;justify-content:center;gap:8px;min-height:44px;padding:0 17px;
  border-radius:12px;border:1px solid transparent;font-size:13px;font-weight:600;white-space:nowrap;
  transition:transform .15s,box-shadow .15s,background .15s,border-color .15s;user-select:none}
.btn:active{transform:translateY(1px)}
.btn:disabled{opacity:.55;cursor:not-allowed;transform:none}
.btn.primary{background:linear-gradient(135deg,#5eead4,#60a5fa);color:#04121c;
  box-shadow:0 14px 34px rgba(94,234,212,.22)}
.btn.primary:hover:not(:disabled){box-shadow:0 18px 40px rgba(94,234,212,.3);transform:translateY(-1px)}
.btn.ghost{background:rgba(13,23,39,.7);border-color:var(--line-2);color:var(--soft)}
.btn.ghost:hover:not(:disabled){border-color:rgba(94,234,212,.45);color:var(--text);background:rgba(16,30,50,.9)}
.btn.danger{background:rgba(251,113,133,.1);border-color:rgba(251,113,133,.3);color:#ffb3c0}
.btn.danger:hover:not(:disabled){background:rgba(251,113,133,.16);border-color:rgba(251,113,133,.5)}
.btn.square{width:44px;min-width:44px;padding:0;flex:0 0 44px}
.btn svg{width:17px;height:17px;display:block}
.btn-row{display:flex;gap:10px;flex-wrap:wrap}
.scan-panel{margin-top:9px;padding:6px;border-radius:14px;background:#070e19;border:1px solid var(--line-2);
  max-height:238px;overflow:auto;box-shadow:inset 0 8px 24px rgba(0,0,0,.22)}
.scan-panel[hidden]{display:none}
.scan-empty{padding:15px 10px;text-align:center;color:var(--dim);font-size:12px}
.scan-item{display:flex;align-items:center;gap:11px;width:100%;padding:10px;border:0;border-radius:10px;
  background:transparent;color:var(--text);text-align:left}
.scan-item:hover{background:rgba(94,234,212,.065)}
.scan-ssid{flex:1;min-width:0;font-size:13px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.scan-meta{display:flex;align-items:center;gap:7px;color:var(--dim);font-size:10.5px;flex:0 0 auto}
.scan-meta svg{width:13px;height:13px;display:block;color:var(--dim)}
.bars{display:flex;align-items:flex-end;gap:2px;height:13px}
.bars i{display:block;width:3px;border-radius:1px;background:#2b3d57}
.bars i.on{background:var(--accent)}
.seg{display:grid;grid-template-columns:1fr 1fr;gap:4px;height:44px;padding:4px;border-radius:12px;
  background:#080f1c;border:1px solid var(--line-2)}
.seg input{position:absolute;opacity:0;width:0;height:0;pointer-events:none}
.seg label{display:flex;align-items:center;justify-content:center;border-radius:9px;font-size:12px;
  color:var(--muted);cursor:pointer;transition:.18s;margin:0}
.seg input:checked+label{color:var(--accent);background:linear-gradient(135deg,rgba(94,234,212,.16),rgba(96,165,250,.12));
  box-shadow:inset 0 0 0 1px rgba(94,234,212,.26)}
.switch{appearance:none;-webkit-appearance:none;width:42px;height:24px;flex:0 0 42px;border-radius:999px;
  background:#1a2942;position:relative;cursor:pointer;transition:.2s;border:0;padding:0;margin:0}
.switch:after{content:"";position:absolute;width:18px;height:18px;border-radius:50%;background:#8fa0bc;
  left:3px;top:3px;transition:.2s;box-shadow:0 2px 5px rgba(0,0,0,.3)}
.switch:checked{background:rgba(94,234,212,.28)}
.switch:checked:after{transform:translateX(18px);background:var(--accent)}
.switch-row{display:flex;align-items:center;gap:10px;height:44px;margin:0;font-size:12px;color:var(--muted);cursor:pointer}
.checkline{display:flex;align-items:center;gap:10px;margin:4px 0 0;font-size:12px;color:var(--muted);cursor:pointer;line-height:1.45}
.checkline input{appearance:none;-webkit-appearance:none;width:18px;height:18px;flex:0 0 18px;border-radius:6px;
  border:1px solid #2a3e5e;background:#080f1c;position:relative;cursor:pointer;padding:0;margin:0}
.checkline input:checked{background:var(--accent);border-color:var(--accent)}
.checkline input:checked:after{content:"";position:absolute;left:5px;top:2px;width:5px;height:9px;
  border:solid #04121c;border-width:0 2px 2px 0;transform:rotate(45deg)}
.range{appearance:none;-webkit-appearance:none;width:100%;height:6px;padding:0;border:0;border-radius:999px;
  background:linear-gradient(90deg,var(--accent) 0%,var(--accent) var(--fill,85%),#1a2942 var(--fill,85%),#1a2942 100%);
  outline:none;cursor:pointer}
.range::-webkit-slider-thumb{appearance:none;-webkit-appearance:none;width:18px;height:18px;border-radius:50%;
  background:#f2f7ff;border:3px solid #0a1220;box-shadow:0 0 0 2px rgba(94,234,212,.7),0 4px 12px rgba(0,0,0,.5);cursor:pointer}
.range::-moz-range-thumb{width:14px;height:14px;border-radius:50%;background:#f2f7ff;border:3px solid #0a1220;
  box-shadow:0 0 0 2px rgba(94,234,212,.7),0 4px 12px rgba(0,0,0,.5);cursor:pointer}
.range:focus-visible{box-shadow:0 0 0 4px rgba(94,234,212,.14)}
.info-list{display:flex;flex-direction:column;gap:9px;margin-bottom:18px}
.info-list>div{display:flex;align-items:center;justify-content:space-between;gap:14px;padding-bottom:9px;
  border-bottom:1px dashed rgba(148,163,184,.12);font-size:12px}
.info-list>div:last-child{padding-bottom:0;border-bottom:0}
.info-list span{color:var(--dim);white-space:nowrap}
.info-list b{font-weight:600;color:var(--soft);font-family:ui-monospace,SFMono-Regular,Consolas,monospace;
  font-size:11.5px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;text-align:right}
.savebar{position:sticky;bottom:16px;z-index:30;display:flex;align-items:center;justify-content:space-between;gap:16px;
  margin-top:18px;padding:14px 15px 14px 20px;border-radius:18px;background:rgba(9,16,28,.9);
  border:1px solid rgba(94,234,212,.2);box-shadow:0 22px 54px rgba(0,0,0,.55);
  backdrop-filter:blur(20px);-webkit-backdrop-filter:blur(20px)}
.save-info{display:flex;align-items:center;gap:10px;min-width:0;font-size:12px;color:var(--muted)}
.save-dot{width:8px;height:8px;flex:0 0 auto;border-radius:50%;background:var(--dim);
  box-shadow:0 0 0 4px rgba(107,124,150,.1);transition:.2s}
.save-dot.dirty{background:var(--amber);box-shadow:0 0 0 4px rgba(251,191,36,.12)}
.save-dot.saved{background:var(--green);box-shadow:0 0 0 4px rgba(52,211,153,.12)}
#saveBtn{min-width:138px}
.spinner{display:none;width:15px;height:15px;border-radius:50%;border:2px solid rgba(4,18,28,.25);
  border-top-color:#04121c;animation:spin .7s linear infinite}
.saving .spinner{display:block}
.saving .btn-label{opacity:.75}
@keyframes spin{to{transform:rotate(360deg)}}
footer{display:flex;justify-content:space-between;gap:16px;flex-wrap:wrap;margin-top:20px;padding:0 4px;
  color:var(--dim);font-size:11px;letter-spacing:.04em}
.toast{position:fixed;left:50%;bottom:28px;z-index:120;display:flex;align-items:center;gap:9px;
  max-width:min(92vw,520px);padding:12px 17px;border-radius:14px;background:#0d1a2c;
  border:1px solid rgba(94,234,212,.32);color:var(--text);font-size:13px;line-height:1.45;
  box-shadow:0 18px 44px rgba(0,0,0,.55);opacity:0;pointer-events:none;
  transform:translate(-50%,18px);transition:opacity .22s,transform .22s}
.toast.show{opacity:1;transform:translate(-50%,0)}
.toast.error{border-color:rgba(251,113,133,.5);color:#ffc7d1}
.toast svg{width:17px;height:17px;flex:0 0 auto}
.reboot{position:fixed;inset:0;z-index:110;display:grid;place-items:center;padding:24px;
  background:rgba(4,8,15,.8);backdrop-filter:blur(14px);-webkit-backdrop-filter:blur(14px)}
.reboot[hidden]{display:none}
.reboot-card{width:min(420px,100%);padding:30px 26px;text-align:center;border-radius:22px;
  background:linear-gradient(168deg,rgba(20,32,52,.98),rgba(8,14,24,.98));
  border:1px solid rgba(94,234,212,.25);box-shadow:0 30px 80px rgba(0,0,0,.6)}
.spinner.big{display:block;width:46px;height:46px;margin:0 auto 18px;border-width:3px;
  border-color:rgba(94,234,212,.16);border-top-color:var(--accent)}
.reboot-card h3{margin:0 0 10px;font-size:18px}
.reboot-card p{margin:0;color:var(--muted);font-size:12.5px;line-height:1.7}
.reboot-hint{margin-top:14px;padding:11px 13px;border-radius:11px;background:rgba(94,234,212,.06);
  border:1px solid rgba(94,234,212,.16);color:var(--soft);font-size:11.5px;line-height:1.65}
.reboot-hint[hidden]{display:none}
dialog{width:min(420px,calc(100vw - 36px));padding:0;border:1px solid rgba(148,163,184,.2);
  border-radius:20px;background:linear-gradient(168deg,#142034,#0a111d);color:var(--text);
  box-shadow:0 30px 80px rgba(0,0,0,.65)}
dialog::backdrop{background:rgba(3,7,13,.72);backdrop-filter:blur(8px)}
.dlg-body{padding:24px 24px 18px}
.dlg-body h3{margin:0 0 9px;font-size:17px}
.dlg-body p{margin:0;color:var(--muted);font-size:12.5px;line-height:1.7}
.dlg-actions{display:flex;justify-content:flex-end;gap:10px;padding:14px 18px 18px}
.reveal{animation:rise .62s cubic-bezier(.2,.8,.2,1) both;animation-delay:var(--d,0s)}
@keyframes rise{from{opacity:0;transform:translateY(14px)}to{opacity:1;transform:none}}
@media(max-width:860px){
  .wrap{padding:24px 15px 112px}
  .overview{grid-template-columns:1fr;gap:20px;padding:22px}
  .grid{grid-template-columns:1fr}
  .three{grid-template-columns:1fr}
  .photo-layout{grid-template-columns:1fr}
  .photo-gallery{grid-template-columns:repeat(2,1fr)}
  .savebar{position:fixed;left:12px;right:12px;bottom:12px;margin:0;padding:12px 12px 12px 16px}
  .save-info{font-size:11.5px}
  #saveBtn{min-width:128px}
  footer{padding-bottom:56px}
}
@media(max-width:520px){
  .top{align-items:flex-start}
  .logo{width:46px;height:46px;border-radius:14px}
  .logo svg{width:29px;height:29px}
  .brand p{font-size:11px}
  .panel{padding:18px;border-radius:18px}
  .overview{padding:19px}
  .two{grid-template-columns:1fr}
  .stats{grid-template-columns:1fr 1fr}
  .device-clock{font-size:57px}
  .save-info span:last-child{white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
}
@media(prefers-reduced-motion:reduce){
  *,*:before,*:after{animation-duration:.001ms!important;animation-iteration-count:1!important;transition-duration:.001ms!important}
}
</style>
</head>
<body>
<div class="aurora" aria-hidden="true"></div>

<main class="wrap">
  <header class="top">
    <div class="brand">
      <div class="logo" aria-hidden="true">
        <svg viewBox="0 0 48 48" fill="none">
          <circle cx="24" cy="24" r="17" stroke="currentColor" stroke-width="2.4" opacity=".9"/>
          <path d="M24 13.5v11l7.5 4.2" stroke="currentColor" stroke-width="2.6" stroke-linecap="round" stroke-linejoin="round"/>
          <circle cx="24" cy="24" r="2.4" fill="currentColor"/>
        </svg>
      </div>
      <div>
        <h1>ESP32-C3 <span>桌面时钟</span></h1>
        <p>ST7735S · 160×128 · 配网与显示设置</p>
      </div>
    </div>
    <div class="status">
      <div class="pill"><span class="dot" id="statusDot"></span><span id="statusText">正在连接</span></div>
      <div class="pill ip" id="ipPill">--</div>
    </div>
  </header>

  <div class="banner" id="apBanner" hidden>
    <strong>配网模式</strong>
    <span>已开启热点 <code id="apBannerSsid">ESP32-Clock</code>。请用手机连接该热点，然后访问 <code>192.168.4.1</code> 完成设置。</span>
  </div>

  <section class="panel overview reveal">
    <div>
      <div class="eyebrow">DEVICE TIME</div>
      <div class="device-clock"><span id="clockHM">--:--</span><span id="deviceSec">--</span></div>
      <div class="device-date" id="deviceDate">正在读取设备状态…</div>
    </div>
    <div class="stats">
      <div class="stat"><span>Wi-Fi</span><strong id="statSsid">--</strong></div>
      <div class="stat"><span>信号</span><strong id="statRssi">--</strong></div>
      <div class="stat"><span>运行时间</span><strong id="statUptime">--</strong></div>
      <div class="stat"><span>空闲内存</span><strong id="statHeap">--</strong></div>
    </div>
  </section>

  <div class="grid">
    <section class="panel reveal" style="--d:.05s">
      <div class="panel-head">
        <div class="ico" aria-hidden="true">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
            <path d="M5 12.5a10 10 0 0 1 14 0"/><path d="M8.5 16a5.5 5.5 0 0 1 7 0"/>
            <circle cx="12" cy="19" r="1" fill="currentColor" stroke="none"/>
          </svg>
        </div>
        <div><h2>网络连接</h2><p>仅支持 2.4 GHz Wi-Fi，保存后设备自动重启</p></div>
        <span class="step">01</span>
      </div>

      <div class="field">
        <label for="ssid">Wi-Fi 名称</label>
        <div class="input-row">
          <input id="ssid" type="text" maxlength="32" autocomplete="off" autocapitalize="none" spellcheck="false" placeholder="选择或输入 Wi-Fi 名称">
          <button id="scanBtn" class="btn ghost square" type="button" title="扫描附近 Wi-Fi">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
              <path d="M21 12a9 9 0 1 1-3-6.7"/><path d="M21 3v6h-6"/>
            </svg>
          </button>
        </div>
        <div class="scan-panel" id="scanPanel" hidden></div>
      </div>

      <div class="field">
        <label for="pass">Wi-Fi 密码</label>
        <div class="input-row">
          <input id="pass" type="password" maxlength="63" autocomplete="new-password" placeholder="留空则保持原密码">
          <button id="eyeBtn" class="btn ghost square" type="button" title="显示密码">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
              <path d="M2 12s3.5-6 10-6 10 6 10 6-3.5 6-10 6S2 12 2 12Z"/><circle cx="12" cy="12" r="2.6"/>
            </svg>
          </button>
        </div>
        <p class="hint" id="passHint">密码仅保存在设备本地 NVS，不会上传到云端。</p>
      </div>

      <label class="checkline"><input type="checkbox" id="openNet"><span>开放网络（无密码）</span></label>
    </section>

    <section class="panel reveal" style="--d:.1s">
      <div class="panel-head">
        <div class="ico" aria-hidden="true">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
            <circle cx="12" cy="12" r="8.5"/><path d="M12 7.5V12l3 2"/>
          </svg>
        </div>
        <div><h2>时间与对时</h2><p>时区、NTP 服务器与表盘显示格式</p></div>
        <span class="step">02</span>
      </div>

      <div class="field">
        <label for="tzPreset">常用时区</label>
        <select id="tzPreset">
          <option value="CST-8">中国 · UTC+8</option>
          <option value="HKT-8">香港 · UTC+8</option>
          <option value="SGT-8">新加坡 · UTC+8</option>
          <option value="JST-9">日本 · UTC+9</option>
          <option value="KST-9">韩国 · UTC+9</option>
          <option value="AEST-10AEDT,M10.1.0,M4.1.0/3">悉尼 · UTC+10 / +11</option>
          <option value="NZST-12NZDT,M9.5.0,M4.1.0/3">奥克兰 · UTC+12 / +13</option>
          <option value="UTC0">UTC · 协调世界时</option>
          <option value="GMT0BST,M3.5.0/1,M10.5.0">伦敦 · UTC+0 / +1</option>
          <option value="CET-1CEST,M3.5.0,M10.5.0/3">中欧 · UTC+1 / +2</option>
          <option value="MSK-3">莫斯科 · UTC+3</option>
          <option value="IST-5:30">印度 · UTC+5:30</option>
          <option value="EST5EDT,M3.2.0,M11.1.0">纽约 · UTC-5 / -4</option>
          <option value="CST6CDT,M3.2.0,M11.1.0">芝加哥 · UTC-6 / -5</option>
          <option value="MST7MDT,M3.2.0,M11.1.0">丹佛 · UTC-7 / -6</option>
          <option value="PST8PDT,M3.2.0,M11.1.0">洛杉矶 · UTC-8 / -7</option>
          <option value="custom">自定义…</option>
        </select>
      </div>

      <div class="field">
        <label for="tz">POSIX 时区字符串</label>
        <input id="tz" type="text" maxlength="63" spellcheck="false" placeholder="CST-8">
        <p class="hint">中国标准时间为 <code>CST-8</code>；一般保持默认即可。</p>
      </div>

      <div class="two">
        <div class="field"><label for="ntp1">主 NTP 服务器</label><input id="ntp1" type="text" maxlength="63" spellcheck="false" placeholder="ntp.aliyun.com"></div>
        <div class="field"><label for="ntp2">备用 NTP 服务器</label><input id="ntp2" type="text" maxlength="63" spellcheck="false" placeholder="cn.pool.ntp.org"></div>
      </div>

      <div class="two">
        <div class="field">
          <label>时间格式</label>
          <div class="seg">
            <input type="radio" name="hour" id="h24" value="24">
            <label for="h24">24 小时</label>
            <input type="radio" name="hour" id="h12" value="12">
            <label for="h12">12 小时</label>
          </div>
        </div>
        <div class="field">
          <label>秒显示</label>
          <label class="switch-row"><input type="checkbox" id="showSeconds" class="switch"><span>显示秒环</span></label>
        </div>
      </div>
    </section>

    <section class="panel reveal" style="--d:.15s">
      <div class="panel-head">
        <div class="ico" aria-hidden="true">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
            <rect x="2.5" y="4.5" width="19" height="13" rx="2"/><path d="M8 21h8M12 17.5V21"/>
          </svg>
        </div>
        <div><h2>屏幕与显示</h2><p>亮度、方向、颜色顺序与像素偏移</p></div>
        <span class="step">03</span>
      </div>

      <div class="field">
        <div class="label-row"><label for="brightness">背光亮度</label><span class="value" id="brightnessVal">85%</span></div>
        <input id="brightness" class="range" type="range" min="5" max="100" step="1" value="85">
        <p class="hint">拖动即可实时预览；点击保存后写入设备。</p>
      </div>

      <div class="two">
        <div class="field">
          <label for="rotation">屏幕方向</label>
          <select id="rotation">
            <option value="1">横屏 · 正常</option>
            <option value="3">横屏 · 翻转 180°</option>
          </select>
        </div>
        <div class="field">
          <label for="colorOrder">颜色顺序</label>
          <select id="colorOrder">
            <option value="bgr">BGR · 大多数模块</option>
            <option value="rgb">RGB</option>
          </select>
        </div>
      </div>

      <div class="two">
        <div class="field"><label for="offsetX">水平偏移</label><input id="offsetX" type="number" min="0" max="8" step="1" value="2"></div>
        <div class="field"><label for="offsetY">垂直偏移</label><input id="offsetY" type="number" min="0" max="8" step="1" value="1"></div>
      </div>

      <div class="field">
        <label for="refreshMode">屏幕刷新率</label>
        <select id="refreshMode">
          <option value="0">标准 · 33 FPS（10 MHz）</option>
          <option value="1">流畅 · 50 FPS（20 MHz）</option>
          <option value="2">极速 · 60 FPS（40 MHz）</option>
        </select>
        <p class="hint">档位越高翻页和动图越顺滑；若出现花屏或闪烁，请调回标准。</p>
      </div>

      <label class="checkline"><input type="checkbox" id="invert"><span>反色显示（颜色像底片时开启）</span></label>
      <p class="hint" style="margin-top:8px">方向、颜色、偏移、刷新率在保存重启后生效。默认偏移 <code>2 / 1</code> 适用于常见 1.8 英寸 ST7735S。</p>
    </section>

    <section class="panel reveal" style="--d:.2s">
      <div class="panel-head">
        <div class="ico" aria-hidden="true">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
            <circle cx="12" cy="12" r="9"/><path d="M12 11v5M12 7.5h.01"/>
          </svg>
        </div>
        <div><h2>设备信息</h2><p>固件、网络地址与维护操作</p></div>
        <span class="step">04</span>
      </div>

      <div class="info-list">
        <div><span>固件版本</span><b id="infoVersion">--</b></div>
        <div><span>芯片</span><b id="infoChip">ESP32-C3</b></div>
        <div><span>MAC 地址</span><b id="infoMac">--</b></div>
        <div><span>配网热点</span><b id="infoAp">--</b></div>
        <div><span>局域网地址</span><b id="infoMdns">esp32clock.local</b></div>
      </div>

      <div class="btn-row">
        <button id="restartBtn" class="btn ghost" type="button">重启设备</button>
        <button id="factoryBtn" class="btn danger" type="button">恢复出厂</button>
      </div>
      <p class="hint" style="margin-top:12px">恢复出厂会清除 Wi-Fi、时区、显示和天气设置，并重新进入配网模式。</p>
    </section>

    <section class="panel reveal" style="--d:.22s;grid-column:1/-1">
      <div class="panel-head">
        <div class="ico" aria-hidden="true">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
            <circle cx="12" cy="10" r="4"/><path d="M12 2.5V5M12 15v2.5M4.5 10H7M17 10h2.5M6.5 4.5l1.8 1.8M15.7 13.7l1.8 1.8M17.5 4.5l-1.8 1.8M8.3 13.7l-1.8 1.8"/><path d="M5 19h14"/>
          </svg>
        </div>
        <div><h2>天气 · 心知天气</h2><p>时钟页与天气页自动轮播，数据来自心知天气 v3</p></div>
        <span class="step">05</span>
      </div>

      <div class="three">
        <div class="field">
          <label class="switch-row"><input type="checkbox" id="weatherEnabled" class="switch"><span>启用天气页</span></label>
          <p class="hint">关闭后只显示时钟页。</p>
        </div>
        <div class="field">
          <label for="weatherKey">心知天气 API Key</label>
          <input id="weatherKey" type="password" autocomplete="off" placeholder="填写心知天气 v3 私钥">
          <p class="hint">已保存的 Key 不会回显；留空保存则保持原 Key。</p>
        </div>
        <div class="field">
          <label for="weatherLocation">城市 / Location</label>
          <input id="weatherLocation" type="text" maxlength="31" placeholder="beijing 或 北京">
          <p class="hint">支持 <code>beijing</code>、城市 ID（如 <code>WX4FBXXFKE4F</code>）或中文城市名。</p>
        </div>
      </div>

      <div class="three" style="margin-top:12px">
        <div class="field">
          <label for="weatherUpdate">天气更新间隔</label>
          <select id="weatherUpdate">
            <option value="10">每 10 分钟</option>
            <option value="15">每 15 分钟</option>
            <option value="30">每 30 分钟</option>
            <option value="60">每 60 分钟</option>
          </select>
        </div>
        <div class="field">
          <label for="weatherPage">时钟 / 天气轮播间隔</label>
          <select id="weatherPage">
            <option value="5">每 5 秒切页</option>
            <option value="10">每 10 秒切页</option>
            <option value="15">每 15 秒切页</option>
            <option value="30">每 30 秒切页</option>
          </select>
        </div>
        <div class="field">
          <label>天气状态</label>
          <div class="hint" id="weatherState" style="min-height:34px">尚未读取</div>
          <button id="weatherRefresh" class="btn ghost" type="button" style="min-height:36px;padding:8px 12px;font-size:12px">立即更新天气</button>
        </div>
      </div>

      <div class="two" style="margin-top:12px">
        <div class="field">
          <label for="weatherLayout">3 天天气布局</label>
          <select id="weatherLayout">
            <option value="0">方案 A · 独立 3 天预报页</option>
            <option value="1">方案 B · 合并到天气页</option>
          </select>
        </div>
        <div class="field">
          <label>布局说明</label>
          <div class="hint" style="min-height:34px">A：时钟 → 天气 → 3天预报 → 动图。B：时钟 → 天气（含3天） → 动图。两种都不显示底部页码点。</div>
        </div>
      </div>
    </section>

    <section class="panel reveal" style="--d:.22s;grid-column:1/-1">
      <div class="panel-head">
        <div class="ico" aria-hidden="true">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
            <rect x="3" y="3" width="7" height="7" rx="1.4"/><rect x="14" y="3" width="7" height="7" rx="1.4"/><rect x="3" y="14" width="7" height="7" rx="1.4"/><rect x="14" y="14" width="7" height="7" rx="1.4"/>
          </svg>
        </div>
        <div><h2>页面与常驻</h2><p>选择显示哪些页面、调整轮播顺序，或指定一个常驻页不再轮播</p></div>
        <span class="step">06</span>
      </div>

      <div class="three">
        <div class="field">
          <label>显示页面</label>
          <label class="check"><input id="pageShowClock" type="checkbox"><span>时钟</span></label>
          <label class="check"><input id="pageShowWeather" type="checkbox"><span>天气</span></label>
          <label class="check"><input id="pageShowForecast" type="checkbox"><span>3 天预报（仅 A 布局）</span></label>
          <label class="check"><input id="pageShowPhoto" type="checkbox"><span>动图</span></label>
        </div>
        <div class="field">
          <label>页面顺序</label>
          <div style="display:flex;align-items:center;gap:8px;margin-bottom:6px">
            <span style="width:64px">时钟</span>
            <select id="pageOrderClock" style="flex:1"><option value="0">第 1 位</option><option value="1">第 2 位</option><option value="2">第 3 位</option><option value="3">第 4 位</option></select>
          </div>
          <div style="display:flex;align-items:center;gap:8px;margin-bottom:6px">
            <span style="width:64px">天气</span>
            <select id="pageOrderWeather" style="flex:1"><option value="0">第 1 位</option><option value="1">第 2 位</option><option value="2">第 3 位</option><option value="3">第 4 位</option></select>
          </div>
          <div style="display:flex;align-items:center;gap:8px;margin-bottom:6px">
            <span style="width:64px">3天预报</span>
            <select id="pageOrderForecast" style="flex:1"><option value="0">第 1 位</option><option value="1">第 2 位</option><option value="2">第 3 位</option><option value="3">第 4 位</option></select>
          </div>
          <div style="display:flex;align-items:center;gap:8px">
            <span style="width:64px">动图</span>
            <select id="pageOrderPhoto" style="flex:1"><option value="0">第 1 位</option><option value="1">第 2 位</option><option value="2">第 3 位</option><option value="3">第 4 位</option></select>
          </div>
          <p class="hint">顺序 1–4 最好各用一次；重复时设备会自动改成不重复的顺序。</p>
        </div>
        <div class="field">
          <label for="residentPage">常驻页</label>
          <select id="residentPage">
            <option value="0">不常驻 · 正常轮播</option>
            <option value="1">常驻时钟</option>
            <option value="2">常驻天气</option>
            <option value="3">常驻 3 天预报</option>
            <option value="4">常驻动图</option>
          </select>
          <p class="hint">常驻页会一直显示，不再自动轮播；它优先于上面的显示开关。设置“不常驻”恢复轮播。全部页面关闭时会自动保留时钟。</p>
        </div>
      </div>
    </section>

    <section class="panel reveal" style="--d:.25s;grid-column:1/-1">
      <div class="panel-head">
        <div class="ico" aria-hidden="true">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">
            <rect x="3" y="4" width="18" height="16" rx="2"/><circle cx="8.5" cy="9" r="1.8"/><path d="M3.5 17l5.2-4.4 4.1 3.4 3.2-2.6 4.5 3.6"/>
          </svg>
        </div>
        <div><h2>照片 · 动图</h2><p>GIF 最多 16 帧；静态图片也可，浏览器逐帧裁切为 160×128 无损 RGB565</p></div>
        <span class="step">07</span>
      </div>

      <div class="photo-layout">
        <div>
          <div class="photo-canvas-wrap">
            <canvas id="photoCanvas" width="160" height="128"></canvas>
          </div>
          <p class="hint" id="photoInfo" style="margin-top:8px">选择照片后自动按 5:4 裁切到 160×128。</p>
        </div>

        <div>
          <div class="photo-drop" id="photoDrop">
            <b>拖入照片，或点击选择</b>
            <span>支持 JPG / PNG / WebP；原图不上传，只在浏览器里处理</span>
            <input id="photoFile" type="file" accept="image/*" hidden>
          </div>

          <div class="two" style="margin-top:12px">
            <div class="field">
              <label>上传格式</label>
              <div class="hint" style="min-height:34px">160×128 · RGB565<br>静态 40 KB / 动图最多 16 帧</div>
              <p class="hint">GIF 在浏览器里逐帧解码，设备只播放 RGB565 帧。</p>
            </div>
            <div class="field">
              <label>压缩结果</label>
              <div class="hint" id="photoSize" style="min-height:34px">等待选择照片/动图</div>
              <p class="hint">16 帧约 640 KB；设备 SPIFFS 共 896 KB。</p>
            </div>
          </div>

          <div class="photo-actions">
            <button id="photoUpload" class="btn primary" type="button" disabled>上传到设备</button>
            <button id="photoClear" class="btn ghost" type="button">清除选择</button>
          </div>
        </div>
      </div>

      <div class="two" style="margin-top:14px">
        <div class="field">
          <label for="photoPageSec">照片页停留时间</label>
          <select id="photoPageSec">
            <option value="10">10 秒</option>
            <option value="15">15 秒</option>
            <option value="30">30 秒</option>
            <option value="60">60 秒</option>
          </select>
        </div>
        <div class="field">
          <label>相册状态</label>
          <div class="hint" id="photoStorage" style="min-height:34px">正在读取…</div>
        </div>
      </div>

      <div class="three" style="margin-top:12px">
        <div class="field">
          <label for="photoTimeStyle">时间样式</label>
          <label class="check" style="min-height:32px;margin-bottom:4px">
            <input id="photoTimeOn" type="checkbox">
            <span>显示时间</span>
          </label>
          <select id="photoTimeStyle">
            <option value="0">大号左下（44px）</option>
            <option value="1">中号左下（30px）</option>
            <option value="2">小号右下（22px）</option>
          </select>
        </div>
        <div class="field">
          <label for="photoTimeColor">时间颜色</label>
          <div style="display:flex;gap:8px;align-items:center">
            <input id="photoTimeColor" type="color" value="#F2F7FF" style="width:52px;height:38px;padding:2px">
            <input id="photoTimeHex" type="text" value="#F2F7FF" spellcheck="false" style="flex:1;min-width:0">
          </div>
        </div>
        <div class="field">
          <label for="photoColon">冒号样式</label>
          <select id="photoColon">
            <option value="0">原版（DIN 方点）</option>
            <option value="1">紧凑圆点</option>
            <option value="2">细竖条</option>
            <option value="3">不要冒号</option>
          </select>
        </div>
      </div>

      <div class="three" style="margin-top:12px">
        <div class="field">
          <label>底部渐变</label>
          <label class="check" style="min-height:38px">
            <input id="photoGradOn" type="checkbox">
            <span>启用渐变</span>
          </label>
        </div>
        <div class="field">
          <label for="photoGradColor">渐变颜色</label>
          <div style="display:flex;gap:8px;align-items:center">
            <input id="photoGradColor" type="color" value="#000000" style="width:52px;height:38px;padding:2px">
            <input id="photoGradHex" type="text" value="#000000" spellcheck="false" style="flex:1;min-width:0">
          </div>
        </div>
        <div class="field">
          <label for="photoGradHeight">渐变高度</label>
          <select id="photoGradHeight">
            <option value="48">48 px</option>
            <option value="56">56 px</option>
            <option value="72">72 px</option>
          </select>
        </div>
      </div>

      <div class="two" style="margin-top:12px">
        <div class="field">
          <label for="photoGradOpa">渐变强度 <span id="photoGradOpaVal">60%</span></label>
          <input id="photoGradOpa" type="range" min="0" max="100" step="5" value="60">
        </div>
        <div class="field">
          <label>说明</label>
          <div class="hint" style="min-height:34px">时间样式、颜色、冒号和渐变都保存到设备；照片页不再显示页码点。</div>
        </div>
      </div>

      <div class="photo-gallery" id="photoGallery"></div>
    </section>
  </div>

  <div class="savebar reveal" style="--d:.3s">
    <div class="save-info"><span class="save-dot saved" id="dirtyDot"></span><span id="saveHint">配置已加载，修改后点击保存</span></div>
    <button id="saveBtn" class="btn primary" type="button">
      <span class="spinner" aria-hidden="true"></span><span class="btn-label">保存并重启</span>
    </button>
  </div>

  <footer><span>ESP32-C3 · ST7735S 160×128 · LVGL 8</span><span id="footerVersion">v--</span></footer>
</main>

<div class="toast" id="toast" role="status" aria-live="polite"></div>

<div class="reboot" id="rebootOverlay" hidden>
  <div class="reboot-card">
    <div class="spinner big" aria-hidden="true"></div>
    <h3 id="rebootTitle">正在保存并重启</h3>
    <p id="rebootText">设备会在几秒后重新上线，请不要断开电源。</p>
    <div class="reboot-hint" id="rebootHint" hidden></div>
  </div>
</div>

<dialog id="confirmDlg">
  <form method="dialog">
    <div class="dlg-body"><h3 id="dlgTitle">确认操作</h3><p id="dlgText"></p></div>
    <div class="dlg-actions">
      <button class="btn ghost" value="cancel">取消</button>
      <button class="btn danger" id="dlgOk" value="ok">确定</button>
    </div>
  </form>
</dialog>

<script>
// (c) Dean McNamee <dean@gmail.com>, 2013.
//
// https://github.com/deanm/omggif
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
// omggif is a JavaScript implementation of a GIF 89a encoder and decoder,
// including animation and compression.  It does not rely on any specific
// underlying system, so should run in the browser, Node, or Plask.

"use strict";

function GifWriter(buf, width, height, gopts) {
  var p = 0;

  var gopts = gopts === undefined ? { } : gopts;
  var loop_count = gopts.loop === undefined ? null : gopts.loop;
  var global_palette = gopts.palette === undefined ? null : gopts.palette;

  if (width <= 0 || height <= 0 || width > 65535 || height > 65535)
    throw new Error("Width/Height invalid.");

  function check_palette_and_num_colors(palette) {
    var num_colors = palette.length;
    if (num_colors < 2 || num_colors > 256 ||  num_colors & (num_colors-1)) {
      throw new Error(
          "Invalid code/color length, must be power of 2 and 2 .. 256.");
    }
    return num_colors;
  }

  // - Header.
  buf[p++] = 0x47; buf[p++] = 0x49; buf[p++] = 0x46;  // GIF
  buf[p++] = 0x38; buf[p++] = 0x39; buf[p++] = 0x61;  // 89a

  // Handling of Global Color Table (palette) and background index.
  var gp_num_colors_pow2 = 0;
  var background = 0;
  if (global_palette !== null) {
    var gp_num_colors = check_palette_and_num_colors(global_palette);
    while (gp_num_colors >>= 1) ++gp_num_colors_pow2;
    gp_num_colors = 1 << gp_num_colors_pow2;
    --gp_num_colors_pow2;
    if (gopts.background !== undefined) {
      background = gopts.background;
      if (background >= gp_num_colors)
        throw new Error("Background index out of range.");
      // The GIF spec states that a background index of 0 should be ignored, so
      // this is probably a mistake and you really want to set it to another
      // slot in the palette.  But actually in the end most browsers, etc end
      // up ignoring this almost completely (including for dispose background).
      if (background === 0)
        throw new Error("Background index explicitly passed as 0.");
    }
  }

  // - Logical Screen Descriptor.
  // NOTE(deanm): w/h apparently ignored by implementations, but set anyway.
  buf[p++] = width & 0xff; buf[p++] = width >> 8 & 0xff;
  buf[p++] = height & 0xff; buf[p++] = height >> 8 & 0xff;
  // NOTE: Indicates 0-bpp original color resolution (unused?).
  buf[p++] = (global_palette !== null ? 0x80 : 0) |  // Global Color Table Flag.
             gp_num_colors_pow2;  // NOTE: No sort flag (unused?).
  buf[p++] = background;  // Background Color Index.
  buf[p++] = 0;  // Pixel aspect ratio (unused?).

  // - Global Color Table
  if (global_palette !== null) {
    for (var i = 0, il = global_palette.length; i < il; ++i) {
      var rgb = global_palette[i];
      buf[p++] = rgb >> 16 & 0xff;
      buf[p++] = rgb >> 8 & 0xff;
      buf[p++] = rgb & 0xff;
    }
  }

  if (loop_count !== null) {  // Netscape block for looping.
    if (loop_count < 0 || loop_count > 65535)
      throw new Error("Loop count invalid.")
    // Extension code, label, and length.
    buf[p++] = 0x21; buf[p++] = 0xff; buf[p++] = 0x0b;
    // NETSCAPE2.0
    buf[p++] = 0x4e; buf[p++] = 0x45; buf[p++] = 0x54; buf[p++] = 0x53;
    buf[p++] = 0x43; buf[p++] = 0x41; buf[p++] = 0x50; buf[p++] = 0x45;
    buf[p++] = 0x32; buf[p++] = 0x2e; buf[p++] = 0x30;
    // Sub-block
    buf[p++] = 0x03; buf[p++] = 0x01;
    buf[p++] = loop_count & 0xff; buf[p++] = loop_count >> 8 & 0xff;
    buf[p++] = 0x00;  // Terminator.
  }


  var ended = false;

  this.addFrame = function(x, y, w, h, indexed_pixels, opts) {
    if (ended === true) { --p; ended = false; }  // Un-end.

    opts = opts === undefined ? { } : opts;

    // TODO(deanm): Bounds check x, y.  Do they need to be within the virtual
    // canvas width/height, I imagine?
    if (x < 0 || y < 0 || x > 65535 || y > 65535)
      throw new Error("x/y invalid.")

    if (w <= 0 || h <= 0 || w > 65535 || h > 65535)
      throw new Error("Width/Height invalid.")

    if (indexed_pixels.length < w * h)
      throw new Error("Not enough pixels for the frame size.");

    var using_local_palette = true;
    var palette = opts.palette;
    if (palette === undefined || palette === null) {
      using_local_palette = false;
      palette = global_palette;
    }

    if (palette === undefined || palette === null)
      throw new Error("Must supply either a local or global palette.");

    var num_colors = check_palette_and_num_colors(palette);

    // Compute the min_code_size (power of 2), destroying num_colors.
    var min_code_size = 0;
    while (num_colors >>= 1) ++min_code_size;
    num_colors = 1 << min_code_size;  // Now we can easily get it back.

    var delay = opts.delay === undefined ? 0 : opts.delay;

    // From the spec:
    //     0 -   No disposal specified. The decoder is
    //           not required to take any action.
    //     1 -   Do not dispose. The graphic is to be left
    //           in place.
    //     2 -   Restore to background color. The area used by the
    //           graphic must be restored to the background color.
    //     3 -   Restore to previous. The decoder is required to
    //           restore the area overwritten by the graphic with
    //           what was there prior to rendering the graphic.
    //  4-7 -    To be defined.
    // NOTE(deanm): Dispose background doesn't really work, apparently most
    // browsers ignore the background palette index and clear to transparency.
    var disposal = opts.disposal === undefined ? 0 : opts.disposal;
    if (disposal < 0 || disposal > 3)  // 4-7 is reserved.
      throw new Error("Disposal out of range.");

    var use_transparency = false;
    var transparent_index = 0;
    if (opts.transparent !== undefined && opts.transparent !== null) {
      use_transparency = true;
      transparent_index = opts.transparent;
      if (transparent_index < 0 || transparent_index >= num_colors)
        throw new Error("Transparent color index.");
    }

    if (disposal !== 0 || use_transparency || delay !== 0) {
      // - Graphics Control Extension
      buf[p++] = 0x21; buf[p++] = 0xf9;  // Extension / Label.
      buf[p++] = 4;  // Byte size.

      buf[p++] = disposal << 2 | (use_transparency === true ? 1 : 0);
      buf[p++] = delay & 0xff; buf[p++] = delay >> 8 & 0xff;
      buf[p++] = transparent_index;  // Transparent color index.
      buf[p++] = 0;  // Block Terminator.
    }

    // - Image Descriptor
    buf[p++] = 0x2c;  // Image Seperator.
    buf[p++] = x & 0xff; buf[p++] = x >> 8 & 0xff;  // Left.
    buf[p++] = y & 0xff; buf[p++] = y >> 8 & 0xff;  // Top.
    buf[p++] = w & 0xff; buf[p++] = w >> 8 & 0xff;
    buf[p++] = h & 0xff; buf[p++] = h >> 8 & 0xff;
    // NOTE: No sort flag (unused?).
    // TODO(deanm): Support interlace.
    buf[p++] = using_local_palette === true ? (0x80 | (min_code_size-1)) : 0;

    // - Local Color Table
    if (using_local_palette === true) {
      for (var i = 0, il = palette.length; i < il; ++i) {
        var rgb = palette[i];
        buf[p++] = rgb >> 16 & 0xff;
        buf[p++] = rgb >> 8 & 0xff;
        buf[p++] = rgb & 0xff;
      }
    }

    p = GifWriterOutputLZWCodeStream(
            buf, p, min_code_size < 2 ? 2 : min_code_size, indexed_pixels);

    return p;
  };

  this.end = function() {
    if (ended === false) {
      buf[p++] = 0x3b;  // Trailer.
      ended = true;
    }
    return p;
  };

  this.getOutputBuffer = function() { return buf; };
  this.setOutputBuffer = function(v) { buf = v; };
  this.getOutputBufferPosition = function() { return p; };
  this.setOutputBufferPosition = function(v) { p = v; };
}

// Main compression routine, palette indexes -> LZW code stream.
// |index_stream| must have at least one entry.
function GifWriterOutputLZWCodeStream(buf, p, min_code_size, index_stream) {
  buf[p++] = min_code_size;
  var cur_subblock = p++;  // Pointing at the length field.

  var clear_code = 1 << min_code_size;
  var code_mask = clear_code - 1;
  var eoi_code = clear_code + 1;
  var next_code = eoi_code + 1;

  var cur_code_size = min_code_size + 1;  // Number of bits per code.
  var cur_shift = 0;
  // We have at most 12-bit codes, so we should have to hold a max of 19
  // bits here (and then we would write out).
  var cur = 0;

  function emit_bytes_to_buffer(bit_block_size) {
    while (cur_shift >= bit_block_size) {
      buf[p++] = cur & 0xff;
      cur >>= 8; cur_shift -= 8;
      if (p === cur_subblock + 256) {  // Finished a subblock.
        buf[cur_subblock] = 255;
        cur_subblock = p++;
      }
    }
  }

  function emit_code(c) {
    cur |= c << cur_shift;
    cur_shift += cur_code_size;
    emit_bytes_to_buffer(8);
  }

  // I am not an expert on the topic, and I don't want to write a thesis.
  // However, it is good to outline here the basic algorithm and the few data
  // structures and optimizations here that make this implementation fast.
  // The basic idea behind LZW is to build a table of previously seen runs
  // addressed by a short id (herein called output code).  All data is
  // referenced by a code, which represents one or more values from the
  // original input stream.  All input bytes can be referenced as the same
  // value as an output code.  So if you didn't want any compression, you
  // could more or less just output the original bytes as codes (there are
  // some details to this, but it is the idea).  In order to achieve
  // compression, values greater then the input range (codes can be up to
  // 12-bit while input only 8-bit) represent a sequence of previously seen
  // inputs.  The decompressor is able to build the same mapping while
  // decoding, so there is always a shared common knowledge between the
  // encoding and decoder, which is also important for "timing" aspects like
  // how to handle variable bit width code encoding.
  //
  // One obvious but very important consequence of the table system is there
  // is always a unique id (at most 12-bits) to map the runs.  'A' might be
  // 4, then 'AA' might be 10, 'AAA' 11, 'AAAA' 12, etc.  This relationship
  // can be used for an effecient lookup strategy for the code mapping.  We
  // need to know if a run has been seen before, and be able to map that run
  // to the output code.  Since we start with known unique ids (input bytes),
  // and then from those build more unique ids (table entries), we can
  // continue this chain (almost like a linked list) to always have small
  // integer values that represent the current byte chains in the encoder.
  // This means instead of tracking the input bytes (AAAABCD) to know our
  // current state, we can track the table entry for AAAABC (it is guaranteed
  // to exist by the nature of the algorithm) and the next character D.
  // Therefor the tuple of (table_entry, byte) is guaranteed to also be
  // unique.  This allows us to create a simple lookup key for mapping input
  // sequences to codes (table indices) without having to store or search
  // any of the code sequences.  So if 'AAAA' has a table entry of 12, the
  // tuple of ('AAAA', K) for any input byte K will be unique, and can be our
  // key.  This leads to a integer value at most 20-bits, which can always
  // fit in an SMI value and be used as a fast sparse array / object key.

  // Output code for the current contents of the index buffer.
  var ib_code = index_stream[0] & code_mask;  // Load first input index.
  var code_table = { };  // Key'd on our 20-bit "tuple".

  emit_code(clear_code);  // Spec says first code should be a clear code.

  // First index already loaded, process the rest of the stream.
  for (var i = 1, il = index_stream.length; i < il; ++i) {
    var k = index_stream[i] & code_mask;
    var cur_key = ib_code << 8 | k;  // (prev, k) unique tuple.
    var cur_code = code_table[cur_key];  // buffer + k.

    // Check if we have to create a new code table entry.
    if (cur_code === undefined) {  // We don't have buffer + k.
      // Emit index buffer (without k).
      // This is an inline version of emit_code, because this is the core
      // writing routine of the compressor (and V8 cannot inline emit_code
      // because it is a closure here in a different context).  Additionally
      // we can call emit_byte_to_buffer less often, because we can have
      // 30-bits (from our 31-bit signed SMI), and we know our codes will only
      // be 12-bits, so can safely have 18-bits there without overflow.
      // emit_code(ib_code);
      cur |= ib_code << cur_shift;
      cur_shift += cur_code_size;
      while (cur_shift >= 8) {
        buf[p++] = cur & 0xff;
        cur >>= 8; cur_shift -= 8;
        if (p === cur_subblock + 256) {  // Finished a subblock.
          buf[cur_subblock] = 255;
          cur_subblock = p++;
        }
      }

      if (next_code === 4096) {  // Table full, need a clear.
        emit_code(clear_code);
        next_code = eoi_code + 1;
        cur_code_size = min_code_size + 1;
        code_table = { };
      } else {  // Table not full, insert a new entry.
        // Increase our variable bit code sizes if necessary.  This is a bit
        // tricky as it is based on "timing" between the encoding and
        // decoder.  From the encoders perspective this should happen after
        // we've already emitted the index buffer and are about to create the
        // first table entry that would overflow our current code bit size.
        if (next_code >= (1 << cur_code_size)) ++cur_code_size;
        code_table[cur_key] = next_code++;  // Insert into code table.
      }

      ib_code = k;  // Index buffer to single input k.
    } else {
      ib_code = cur_code;  // Index buffer to sequence in code table.
    }
  }

  emit_code(ib_code);  // There will still be something in the index buffer.
  emit_code(eoi_code);  // End Of Information.

  // Flush / finalize the sub-blocks stream to the buffer.
  emit_bytes_to_buffer(1);

  // Finish the sub-blocks, writing out any unfinished lengths and
  // terminating with a sub-block of length 0.  If we have already started
  // but not yet used a sub-block it can just become the terminator.
  if (cur_subblock + 1 === p) {  // Started but unused.
    buf[cur_subblock] = 0;
  } else {  // Started and used, write length and additional terminator block.
    buf[cur_subblock] = p - cur_subblock - 1;
    buf[p++] = 0;
  }
  return p;
}

function GifReader(buf) {
  var p = 0;

  // - Header (GIF87a or GIF89a).
  if (buf[p++] !== 0x47 ||            buf[p++] !== 0x49 || buf[p++] !== 0x46 ||
      buf[p++] !== 0x38 || (buf[p++]+1 & 0xfd) !== 0x38 || buf[p++] !== 0x61) {
    throw new Error("Invalid GIF 87a/89a header.");
  }

  // - Logical Screen Descriptor.
  var width = buf[p++] | buf[p++] << 8;
  var height = buf[p++] | buf[p++] << 8;
  var pf0 = buf[p++];  // <Packed Fields>.
  var global_palette_flag = pf0 >> 7;
  var num_global_colors_pow2 = pf0 & 0x7;
  var num_global_colors = 1 << (num_global_colors_pow2 + 1);
  var background = buf[p++];
  buf[p++];  // Pixel aspect ratio (unused?).

  var global_palette_offset = null;
  var global_palette_size   = null;

  if (global_palette_flag) {
    global_palette_offset = p;
    global_palette_size = num_global_colors;
    p += num_global_colors * 3;  // Seek past palette.
  }

  var no_eof = true;

  var frames = [ ];

  var delay = 0;
  var transparent_index = null;
  var disposal = 0;  // 0 - No disposal specified.
  var loop_count = null;

  this.width = width;
  this.height = height;

  while (no_eof && p < buf.length) {
    switch (buf[p++]) {
      case 0x21:  // Graphics Control Extension Block
        switch (buf[p++]) {
          case 0xff:  // Application specific block
            // Try if it's a Netscape block (with animation loop counter).
            if (buf[p   ] !== 0x0b ||  // 21 FF already read, check block size.
                // NETSCAPE2.0
                buf[p+1 ] == 0x4e && buf[p+2 ] == 0x45 && buf[p+3 ] == 0x54 &&
                buf[p+4 ] == 0x53 && buf[p+5 ] == 0x43 && buf[p+6 ] == 0x41 &&
                buf[p+7 ] == 0x50 && buf[p+8 ] == 0x45 && buf[p+9 ] == 0x32 &&
                buf[p+10] == 0x2e && buf[p+11] == 0x30 &&
                // Sub-block
                buf[p+12] == 0x03 && buf[p+13] == 0x01 && buf[p+16] == 0) {
              p += 14;
              loop_count = buf[p++] | buf[p++] << 8;
              p++;  // Skip terminator.
            } else {  // We don't know what it is, just try to get past it.
              p += 12;
              while (true) {  // Seek through subblocks.
                var block_size = buf[p++];
                // Bad block size (ex: undefined from an out of bounds read).
                if (!(block_size >= 0)) throw Error("Invalid block size");
                if (block_size === 0) break;  // 0 size is terminator
                p += block_size;
              }
            }
            break;

          case 0xf9:  // Graphics Control Extension
            if (buf[p++] !== 0x4 || buf[p+4] !== 0)
              throw new Error("Invalid graphics extension block.");
            var pf1 = buf[p++];
            delay = buf[p++] | buf[p++] << 8;
            transparent_index = buf[p++];
            if ((pf1 & 1) === 0) transparent_index = null;
            disposal = pf1 >> 2 & 0x7;
            p++;  // Skip terminator.
            break;

          case 0xfe:  // Comment Extension.
            while (true) {  // Seek through subblocks.
              var block_size = buf[p++];
              // Bad block size (ex: undefined from an out of bounds read).
              if (!(block_size >= 0)) throw Error("Invalid block size");
              if (block_size === 0) break;  // 0 size is terminator
              // console.log(buf.slice(p, p+block_size).toString('ascii'));
              p += block_size;
            }
            break;

          default:
            throw new Error(
                "Unknown graphic control label: 0x" + buf[p-1].toString(16));
        }
        break;

      case 0x2c:  // Image Descriptor.
        var x = buf[p++] | buf[p++] << 8;
        var y = buf[p++] | buf[p++] << 8;
        var w = buf[p++] | buf[p++] << 8;
        var h = buf[p++] | buf[p++] << 8;
        var pf2 = buf[p++];
        var local_palette_flag = pf2 >> 7;
        var interlace_flag = pf2 >> 6 & 1;
        var num_local_colors_pow2 = pf2 & 0x7;
        var num_local_colors = 1 << (num_local_colors_pow2 + 1);
        var palette_offset = global_palette_offset;
        var palette_size = global_palette_size;
        var has_local_palette = false;
        if (local_palette_flag) {
          var has_local_palette = true;
          palette_offset = p;  // Override with local palette.
          palette_size = num_local_colors;
          p += num_local_colors * 3;  // Seek past palette.
        }

        var data_offset = p;

        p++;  // codesize
        while (true) {
          var block_size = buf[p++];
          // Bad block size (ex: undefined from an out of bounds read).
          if (!(block_size >= 0)) throw Error("Invalid block size");
          if (block_size === 0) break;  // 0 size is terminator
          p += block_size;
        }

        frames.push({x: x, y: y, width: w, height: h,
                     has_local_palette: has_local_palette,
                     palette_offset: palette_offset,
                     palette_size: palette_size,
                     data_offset: data_offset,
                     data_length: p - data_offset,
                     transparent_index: transparent_index,
                     interlaced: !!interlace_flag,
                     delay: delay,
                     disposal: disposal});
        break;

      case 0x3b:  // Trailer Marker (end of file).
        no_eof = false;
        break;

      default:
        throw new Error("Unknown gif block: 0x" + buf[p-1].toString(16));
        break;
    }
  }

  this.numFrames = function() {
    return frames.length;
  };

  this.loopCount = function() {
    return loop_count;
  };

  this.frameInfo = function(frame_num) {
    if (frame_num < 0 || frame_num >= frames.length)
      throw new Error("Frame index out of range.");
    return frames[frame_num];
  }

  this.decodeAndBlitFrameBGRA = function(frame_num, pixels) {
    var frame = this.frameInfo(frame_num);
    var num_pixels = frame.width * frame.height;
    var index_stream = new Uint8Array(num_pixels);  // At most 8-bit indices.
    GifReaderLZWOutputIndexStream(
        buf, frame.data_offset, index_stream, num_pixels);
    var palette_offset = frame.palette_offset;

    // NOTE(deanm): It seems to be much faster to compare index to 256 than
    // to === null.  Not sure why, but CompareStub_EQ_STRICT shows up high in
    // the profile, not sure if it's related to using a Uint8Array.
    var trans = frame.transparent_index;
    if (trans === null) trans = 256;

    // We are possibly just blitting to a portion of the entire frame.
    // That is a subrect within the framerect, so the additional pixels
    // must be skipped over after we finished a scanline.
    var framewidth  = frame.width;
    var framestride = width - framewidth;
    var xleft       = framewidth;  // Number of subrect pixels left in scanline.

    // Output indicies of the top left and bottom right corners of the subrect.
    var opbeg = ((frame.y * width) + frame.x) * 4;
    var opend = ((frame.y + frame.height) * width + frame.x) * 4;
    var op    = opbeg;

    var scanstride = framestride * 4;

    // Use scanstride to skip past the rows when interlacing.  This is skipping
    // 7 rows for the first two passes, then 3 then 1.
    if (frame.interlaced === true) {
      scanstride += width * 4 * 7;  // Pass 1.
    }

    var interlaceskip = 8;  // Tracking the row interval in the current pass.

    for (var i = 0, il = index_stream.length; i < il; ++i) {
      var index = index_stream[i];

      if (xleft === 0) {  // Beginning of new scan line
        op += scanstride;
        xleft = framewidth;
        if (op >= opend) { // Catch the wrap to switch passes when interlacing.
          scanstride = framestride * 4 + width * 4 * (interlaceskip-1);
          // interlaceskip / 2 * 4 is interlaceskip << 1.
          op = opbeg + (framewidth + framestride) * (interlaceskip << 1);
          interlaceskip >>= 1;
        }
      }

      if (index === trans) {
        op += 4;
      } else {
        var r = buf[palette_offset + index * 3];
        var g = buf[palette_offset + index * 3 + 1];
        var b = buf[palette_offset + index * 3 + 2];
        pixels[op++] = b;
        pixels[op++] = g;
        pixels[op++] = r;
        pixels[op++] = 255;
      }
      --xleft;
    }
  };

  // I will go to copy and paste hell one day...
  this.decodeAndBlitFrameRGBA = function(frame_num, pixels) {
    var frame = this.frameInfo(frame_num);
    var num_pixels = frame.width * frame.height;
    var index_stream = new Uint8Array(num_pixels);  // At most 8-bit indices.
    GifReaderLZWOutputIndexStream(
        buf, frame.data_offset, index_stream, num_pixels);
    var palette_offset = frame.palette_offset;

    // NOTE(deanm): It seems to be much faster to compare index to 256 than
    // to === null.  Not sure why, but CompareStub_EQ_STRICT shows up high in
    // the profile, not sure if it's related to using a Uint8Array.
    var trans = frame.transparent_index;
    if (trans === null) trans = 256;

    // We are possibly just blitting to a portion of the entire frame.
    // That is a subrect within the framerect, so the additional pixels
    // must be skipped over after we finished a scanline.
    var framewidth  = frame.width;
    var framestride = width - framewidth;
    var xleft       = framewidth;  // Number of subrect pixels left in scanline.

    // Output indicies of the top left and bottom right corners of the subrect.
    var opbeg = ((frame.y * width) + frame.x) * 4;
    var opend = ((frame.y + frame.height) * width + frame.x) * 4;
    var op    = opbeg;

    var scanstride = framestride * 4;

    // Use scanstride to skip past the rows when interlacing.  This is skipping
    // 7 rows for the first two passes, then 3 then 1.
    if (frame.interlaced === true) {
      scanstride += width * 4 * 7;  // Pass 1.
    }

    var interlaceskip = 8;  // Tracking the row interval in the current pass.

    for (var i = 0, il = index_stream.length; i < il; ++i) {
      var index = index_stream[i];

      if (xleft === 0) {  // Beginning of new scan line
        op += scanstride;
        xleft = framewidth;
        if (op >= opend) { // Catch the wrap to switch passes when interlacing.
          scanstride = framestride * 4 + width * 4 * (interlaceskip-1);
          // interlaceskip / 2 * 4 is interlaceskip << 1.
          op = opbeg + (framewidth + framestride) * (interlaceskip << 1);
          interlaceskip >>= 1;
        }
      }

      if (index === trans) {
        op += 4;
      } else {
        var r = buf[palette_offset + index * 3];
        var g = buf[palette_offset + index * 3 + 1];
        var b = buf[palette_offset + index * 3 + 2];
        pixels[op++] = r;
        pixels[op++] = g;
        pixels[op++] = b;
        pixels[op++] = 255;
      }
      --xleft;
    }
  };
}

function GifReaderLZWOutputIndexStream(code_stream, p, output, output_length) {
  var min_code_size = code_stream[p++];

  var clear_code = 1 << min_code_size;
  var eoi_code = clear_code + 1;
  var next_code = eoi_code + 1;

  var cur_code_size = min_code_size + 1;  // Number of bits per code.
  // NOTE: This shares the same name as the encoder, but has a different
  // meaning here.  Here this masks each code coming from the code stream.
  var code_mask = (1 << cur_code_size) - 1;
  var cur_shift = 0;
  var cur = 0;

  var op = 0;  // Output pointer.

  var subblock_size = code_stream[p++];

  // TODO(deanm): Would using a TypedArray be any faster?  At least it would
  // solve the fast mode / backing store uncertainty.
  // var code_table = Array(4096);
  var code_table = new Int32Array(4096);  // Can be signed, we only use 20 bits.

  var prev_code = null;  // Track code-1.

  while (true) {
    // Read up to two bytes, making sure we always 12-bits for max sized code.
    while (cur_shift < 16) {
      if (subblock_size === 0) break;  // No more data to be read.

      cur |= code_stream[p++] << cur_shift;
      cur_shift += 8;

      if (subblock_size === 1) {  // Never let it get to 0 to hold logic above.
        subblock_size = code_stream[p++];  // Next subblock.
      } else {
        --subblock_size;
      }
    }

    // TODO(deanm): We should never really get here, we should have received
    // and EOI.
    if (cur_shift < cur_code_size)
      break;

    var code = cur & code_mask;
    cur >>= cur_code_size;
    cur_shift -= cur_code_size;

    // TODO(deanm): Maybe should check that the first code was a clear code,
    // at least this is what you're supposed to do.  But actually our encoder
    // now doesn't emit a clear code first anyway.
    if (code === clear_code) {
      // We don't actually have to clear the table.  This could be a good idea
      // for greater error checking, but we don't really do any anyway.  We
      // will just track it with next_code and overwrite old entries.

      next_code = eoi_code + 1;
      cur_code_size = min_code_size + 1;
      code_mask = (1 << cur_code_size) - 1;

      // Don't update prev_code ?
      prev_code = null;
      continue;
    } else if (code === eoi_code) {
      break;
    }

    // We have a similar situation as the decoder, where we want to store
    // variable length entries (code table entries), but we want to do in a
    // faster manner than an array of arrays.  The code below stores sort of a
    // linked list within the code table, and then "chases" through it to
    // construct the dictionary entries.  When a new entry is created, just the
    // last byte is stored, and the rest (prefix) of the entry is only
    // referenced by its table entry.  Then the code chases through the
    // prefixes until it reaches a single byte code.  We have to chase twice,
    // first to compute the length, and then to actually copy the data to the
    // output (backwards, since we know the length).  The alternative would be
    // storing something in an intermediate stack, but that doesn't make any
    // more sense.  I implemented an approach where it also stored the length
    // in the code table, although it's a bit tricky because you run out of
    // bits (12 + 12 + 8), but I didn't measure much improvements (the table
    // entries are generally not the long).  Even when I created benchmarks for
    // very long table entries the complexity did not seem worth it.
    // The code table stores the prefix entry in 12 bits and then the suffix
    // byte in 8 bits, so each entry is 20 bits.

    var chase_code = code < next_code ? code : prev_code;

    // Chase what we will output, either {CODE} or {CODE-1}.
    var chase_length = 0;
    var chase = chase_code;
    while (chase > clear_code) {
      chase = code_table[chase] >> 8;
      ++chase_length;
    }

    var k = chase;

    var op_end = op + chase_length + (chase_code !== code ? 1 : 0);
    if (op_end > output_length) {
      console.log("Warning, gif stream longer than expected.");
      return;
    }

    // Already have the first byte from the chase, might as well write it fast.
    output[op++] = k;

    op += chase_length;
    var b = op;  // Track pointer, writing backwards.

    if (chase_code !== code)  // The case of emitting {CODE-1} + k.
      output[op++] = k;

    chase = chase_code;
    while (chase_length--) {
      chase = code_table[chase];
      output[--b] = chase & 0xff;  // Write backwards.
      chase >>= 8;  // Pull down to the prefix code.
    }

    if (prev_code !== null && next_code < 4096) {
      code_table[next_code++] = prev_code << 8 | k;
      // TODO(deanm): Figure out this clearing vs code growth logic better.  I
      // have an feeling that it should just happen somewhere else, for now it
      // is awkward between when we grow past the max and then hit a clear code.
      // For now just check if we hit the max 12-bits (then a clear code should
      // follow, also of course encoded in 12-bits).
      if (next_code >= code_mask+1 && cur_code_size < 12) {
        ++cur_code_size;
        code_mask = code_mask << 1 | 1;
      }
    }

    prev_code = code;
  }

  if (op !== output_length) {
    console.log("Warning, gif stream shorter than expected.");
  }

  return output;
}

// CommonJS.
try { exports.GifWriter = GifWriter; exports.GifReader = GifReader } catch(e) {}

</script>

<script>
(function () {
  "use strict";
  var $ = function (id) { return document.getElementById(id); };
  var state = { cfg: null, status: null, dirty: false, saving: false, clockBase: null, poll: null,
    photos: null, photoImg: null, photoBlob: null, photoFile: null, uploadingPhoto: false,
    previewFrames: [], animDelay: 0, animTimer: null };
  var toastTimer = null;

  function setStatus(kind, text) {
    var dot = $("statusDot");
    dot.className = "dot " + (kind || "");
    $("statusText").textContent = text;
  }

  function toast(message, type) {
    var el = $("toast");
    el.textContent = message;
    el.className = "toast show" + (type === "error" ? " error" : "");
    clearTimeout(toastTimer);
    toastTimer = setTimeout(function () { el.className = "toast"; }, 3800);
  }

  async function api(path, options, timeoutMs) {
    options = options || {};
    var ctrl = new AbortController();
    var timer = setTimeout(function () { ctrl.abort(); }, timeoutMs || 8000);
    try {
      var res = await fetch(path, Object.assign({ cache: "no-store", signal: ctrl.signal }, options));
      var text = await res.text();
      var data = {};
      if (text) {
        try { data = JSON.parse(text); }
        catch (e) { data = { error: text }; }
      }
      if (!res.ok || data.ok === false) throw new Error(data.error || ("HTTP " + res.status));
      return data;
    } catch (e) {
      if (e && e.name === "AbortError") throw new Error("请求超时，请检查连接");
      throw e;
    } finally {
      clearTimeout(timer);
    }
  }

  function postForm(path, params) {
    var body = new URLSearchParams(params || {});
    return api(path, {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded;charset=UTF-8" },
      body: body.toString()
    }, 12000);
  }

  function markDirty() {
    if (state.saving) return;
    state.dirty = true;
    $("dirtyDot").className = "save-dot dirty";
    $("saveHint").textContent = "有未保存的修改";
  }

  function markClean(text) {
    state.dirty = false;
    $("dirtyDot").className = "save-dot saved";
    $("saveHint").textContent = text || "配置已保存";
  }

  function setSaving(on) {
    state.saving = on;
    var btn = $("saveBtn");
    btn.disabled = on;
    btn.classList.toggle("saving", on);
    btn.querySelector(".btn-label").textContent = on ? "正在保存…" : "保存并重启";
    if (on) $("saveHint").textContent = "正在写入 NVS 并重启…";
  }

  function fmtUptime(sec) {
    sec = Math.max(0, sec | 0);
    var d = Math.floor(sec / 86400);
    var h = Math.floor((sec % 86400) / 3600);
    var m = Math.floor((sec % 3600) / 60);
    if (d) return d + " 天 " + h + " 小时";
    if (h) return h + " 小时 " + m + " 分";
    if (m) return m + " 分 " + (sec % 60) + " 秒";
    return sec + " 秒";
  }

  function rssiText(rssi) {
    if (!rssi) return "--";
    var q = rssi >= -55 ? "极佳" : rssi >= -67 ? "良好" : rssi >= -78 ? "一般" : "较弱";
    return rssi + " dBm · " + q;
  }

  function setClockBase(timeText) {
    var m = /^(\d{2}):(\d{2}):(\d{2})$/.exec(timeText || "");
    if (!m) { state.clockBase = null; return; }
    state.clockBase = { h: +m[1], m: +m[2], s: +m[3], at: Date.now() };
  }

  function tickClock() {
    var b = state.clockBase;
    if (!b) return;
    var total = b.h * 3600 + b.m * 60 + b.s + Math.floor((Date.now() - b.at) / 1000);
    total = ((total % 86400) + 86400) % 86400;
    var h = Math.floor(total / 3600);
    var m = Math.floor((total % 3600) / 60);
    var s = total % 60;
    $("clockHM").textContent = (h < 10 ? "0" : "") + h + ":" + (m < 10 ? "0" : "") + m;
    $("deviceSec").textContent = (s < 10 ? "0" : "") + s;
  }

  function fillConfig(cfg) {
    state.cfg = cfg;
    $("ssid").value = cfg.ssid || "";
    $("pass").value = "";
    $("pass").placeholder = cfg.pass_set ? "已保存密码，留空则不修改" : "请输入 Wi-Fi 密码";
    $("passHint").textContent = cfg.pass_set
      ? "密码已保存。留空表示不修改；更换 Wi-Fi 时需要重新输入密码。"
      : "密码仅保存在设备本地 NVS，不会上传到云端。";

    $("tz").value = cfg.tz || "CST-8";
    var preset = $("tzPreset");
    var found = false;
    for (var i = 0; i < preset.options.length; i++) {
      if (preset.options[i].value === cfg.tz) { preset.value = cfg.tz; found = true; break; }
    }
    if (!found) preset.value = "custom";

    $("ntp1").value = cfg.ntp1 || "";
    $("ntp2").value = cfg.ntp2 || "";
    $("brightness").value = cfg.brightness;
    $("brightnessVal").textContent = cfg.brightness + "%";
    $("brightness").style.setProperty("--fill", cfg.brightness + "%");
    $("rotation").value = String(cfg.rotation);
    $("colorOrder").value = cfg.color_order;
    $("offsetX").value = cfg.offset_x;
    $("offsetY").value = cfg.offset_y;
    $("refreshMode").value = String(cfg.refresh_mode || 0);
    $("invert").checked = !!cfg.invert;
    $("showSeconds").checked = !!cfg.show_seconds;
    $("weatherEnabled").checked = !!cfg.weather_enabled;
    $("weatherLocation").value = cfg.weather_location || "beijing";
    $("weatherKey").value = "";
    $("weatherKey").placeholder = cfg.weather_key_set ? "已保存 Key，留空不修改" : "填写心知天气 v3 私钥";
    setSelectValue("weatherUpdate", cfg.weather_update_min || 15);
    setSelectValue("weatherPage", cfg.weather_page_sec || 10);
    setSelectValue("weatherLayout", cfg.weather_layout || 0);
    setSelectValue("photoPageSec", cfg.photo_page_sec || 15);
    $("photoTimeOn").checked = cfg.photo_time_on !== false;
    setSelectValue("photoTimeStyle", cfg.photo_time_style || 0);
    $("pageShowClock").checked = !!cfg.page_show_clock;
    $("pageShowWeather").checked = !!cfg.page_show_weather;
    $("pageShowForecast").checked = !!cfg.page_show_forecast;
    $("pageShowPhoto").checked = !!cfg.page_show_photo;
    setSelectValue("pageOrderClock", cfg.page_order_clock == null ? 0 : cfg.page_order_clock);
    setSelectValue("pageOrderWeather", cfg.page_order_weather == null ? 1 : cfg.page_order_weather);
    setSelectValue("pageOrderForecast", cfg.page_order_forecast == null ? 2 : cfg.page_order_forecast);
    setSelectValue("pageOrderPhoto", cfg.page_order_photo == null ? 3 : cfg.page_order_photo);
    setSelectValue("residentPage", cfg.resident_page || 0);
    $("photoTimeColor").value = cfg.photo_time_color || "#F2F7FF";
    $("photoTimeHex").value = (cfg.photo_time_color || "#F2F7FF").toUpperCase();
    setSelectValue("photoColon", cfg.photo_colon_style == null ? 1 : cfg.photo_colon_style);
    $("photoGradOn").checked = !!cfg.photo_grad_on;
    $("photoGradColor").value = cfg.photo_grad_color || "#000000";
    $("photoGradHex").value = (cfg.photo_grad_color || "#000000").toUpperCase();
    setSelectValue("photoGradHeight", cfg.photo_grad_height || 56);
    $("photoGradOpa").value = cfg.photo_grad_opa == null ? 60 : cfg.photo_grad_opa;
    $("photoGradOpaVal").textContent = $("photoGradOpa").value + "%";
    $("h12").checked = !!cfg.twelve_hour;
    $("h24").checked = !cfg.twelve_hour;
    $("infoVersion").textContent = "v" + (cfg.version || "--");
    $("footerVersion").textContent = "v" + (cfg.version || "--");
    $("infoAp").textContent = cfg.ap_ssid || "--";
    $("openNet").checked = !!cfg.open_network;
    setPassDisabled(!!cfg.open_network);
    clearPhotoSelection();
    markClean("配置已加载，修改后点击保存");
  }

  function setSelectValue(id, value) {
    var el = $(id);
    if (!el) return;
    var v = String(value);
    for (var i = 0; i < el.options.length; i++) {
      if (el.options[i].value === v) { el.value = v; return; }
    }
    el.selectedIndex = 0;
  }

  function weatherAgeText(sec) {
    sec = Math.max(0, sec | 0);
    if (sec < 60) return "刚刚更新";
    if (sec < 3600) return Math.floor(sec / 60) + " 分钟前";
    return Math.floor(sec / 3600) + " 小时前";
  }

  function fillWeather(w) {
    var el = $("weatherState");
    if (!el) return;
    if (!w) { el.textContent = "尚未读取"; return; }
    if (!w.valid) {
      el.textContent = w.error || "等待天气数据";
      return;
    }
    el.textContent = (w.city || "本地") + " · " + w.temp + "° " + (w.text || "") +
      " · " + weatherAgeText(w.age);
  }

  function formatBytes(n) {
    n = n || 0;
    if (n < 1024) return n + " B";
    if (n < 1024 * 1024) return (n / 1024).toFixed(1) + " KB";
    return (n / 1024 / 1024).toFixed(2) + " MB";
  }

  function photoMax() { return (state.photos && state.photos.max) || 1; }
  function photoCount() { return (state.photos && state.photos.count) || 0; }

  function updatePhotoButtons() {
    var btn = $("photoUpload");
    if (!btn) return;
    btn.disabled = state.uploadingPhoto || !state.photoBlob ||
      state.photoBlob.size > 700000;
    btn.textContent = state.uploadingPhoto ? "正在上传…" :
      (photoCount() >= 1 ? "替换动图/照片" : "上传到设备");
  }

  function stopAnimPreview() {
    if (state.animTimer) {
      clearInterval(state.animTimer);
      state.animTimer = null;
    }
  }

  function clearPhotoSelection() {
    stopAnimPreview();
    state.photoImg = null;
    state.photoBlob = null;
    state.photoFile = null;
    state.previewFrames = [];
    state.animDelay = 0;
    var input = $("photoFile");
    if (input) input.value = "";
    var canvas = $("photoCanvas");
    if (canvas) {
      var ctx = canvas.getContext("2d");
      ctx.fillStyle = "#02060c";
      ctx.fillRect(0, 0, canvas.width, canvas.height);
      ctx.fillStyle = "#526784";
      ctx.font = "11px sans-serif";
      ctx.textAlign = "center";
      ctx.fillText("160 × 128 预览", canvas.width / 2, canvas.height / 2 + 4);
    }
    $("photoSize").textContent = "等待选择照片/动图";
    $("photoInfo").textContent = "GIF 最多 16 帧；静态图片也可以。";
    updatePhotoButtons();
  }

  function packCanvasRaw(canvas) {
    var ctx = canvas.getContext("2d");
    var src = ctx.getImageData(0, 0, canvas.width, canvas.height).data;
    var raw = new Uint8Array(canvas.width * canvas.height * 2);
    for (var i = 0, j = 0; i < src.length; i += 4, j += 2) {
      var v = ((src[i] >> 3) << 11) | ((src[i + 1] >> 2) << 5) | (src[i + 2] >> 3);
      raw[j] = v & 0xFF;
      raw[j + 1] = (v >> 8) & 0xFF;
    }
    return raw;
  }

  function buildAnimBlob(frames, delay) {
    var header = new Uint8Array(12);
    header.set([65, 78, 73, 77], 0);  // "ANIM"
    header[4] = 1;
    header[5] = frames.length;
    header[6] = delay & 0xFF;
    header[7] = (delay >> 8) & 0xFF;
    var parts = [header];
    for (var i = 0; i < frames.length; i++) parts.push(frames[i]);
    return new Blob(parts, { type: "application/octet-stream" });
  }

  function drawRawPhoto(canvas, buffer) {
    var a = new Uint8Array(buffer);
    if (a.length !== 160 * 128 * 2) return;
    var ctx = canvas.getContext("2d");
    var img = ctx.createImageData(160, 128);
    for (var i = 0, j = 0; i < a.length; i += 2, j += 4) {
      var v = a[i] | (a[i + 1] << 8);
      img.data[j] = ((v >> 11) & 0x1F) * 255 / 31;
      img.data[j + 1] = ((v >> 5) & 0x3F) * 255 / 63;
      img.data[j + 2] = (v & 0x1F) * 255 / 31;
      img.data[j + 3] = 255;
    }
    ctx.putImageData(img, 0, 0);
  }

  function startAnimPreview(frames, delay) {
    stopAnimPreview();
    if (!frames.length) return;
    var canvas = $("photoCanvas");
    var index = 0;
    drawRawPhoto(canvas, frames[0].buffer);
    if (frames.length <= 1 || delay <= 0) return;
    state.animTimer = setInterval(function () {
      index = (index + 1) % frames.length;
      drawRawPhoto(canvas, frames[index].buffer);
    }, delay);
  }

  function setAnimReady(frames, delay, info) {
    state.previewFrames = frames;
    state.animDelay = delay;
    state.photoBlob = buildAnimBlob(frames, delay);
    $("photoSize").textContent = formatBytes(state.photoBlob.size) + " · " +
      frames.length + " 帧" + (delay ? " · " + delay + " ms" : " · 静态");
    $("photoSize").style.color = "var(--accent)";
    $("photoInfo").textContent = info;
    startAnimPreview(frames, delay);
    updatePhotoButtons();
  }

  function renderPhoto() {
    var img = state.photoImg;
    if (!img) return;
    var canvas = document.createElement("canvas");
    canvas.width = 160;
    canvas.height = 128;
    var ctx = canvas.getContext("2d");
    var scale = Math.max(canvas.width / img.width, canvas.height / img.height);
    var sw = canvas.width / scale;
    var sh = canvas.height / scale;
    var sx = (img.width - sw) / 2;
    var sy = (img.height - sh) / 2;
    ctx.imageSmoothingEnabled = true;
    ctx.imageSmoothingQuality = "high";

    // 先放大到 4 倍画布再缩回 160x128，减少直接缩小产生的锯齿。
    var big = document.createElement("canvas");
    big.width = 640;
    big.height = 512;
    var bctx = big.getContext("2d");
    bctx.imageSmoothingEnabled = true;
    bctx.imageSmoothingQuality = "high";
    bctx.drawImage(img, sx, sy, sw, sh, 0, 0, big.width, big.height);
    ctx.drawImage(big, 0, 0, canvas.width, canvas.height);

    var raw = packCanvasRaw(canvas);
    setAnimReady([raw], 0,
      "原图 " + img.width + "×" + img.height + " · " +
      formatBytes(state.photoFile ? state.photoFile.size : 0) +
      " → 4× 超采样 → 160×128 无损 RGB565");
  }

  function selectStill(file) {
    state.photoFile = file;
    var img = new Image();
    img.onload = function () {
      state.photoImg = img;
      renderPhoto();
    };
    img.onerror = function () { toast("图片读取失败", "error"); };
    img.src = URL.createObjectURL(file);
  }

  function selectGifOmggif(file) {
    return new Promise(function (resolve, reject) {
      if (typeof GifReader !== "function") {
        reject(new Error("GIF 解码器未加载"));
        return;
      }
      file.arrayBuffer().then(function (ab) {
        try {
          var reader = new GifReader(new Uint8Array(ab));
          var W = reader.width;
          var H = reader.height;
          var total = reader.numFrames();
          if (!W || !H || !total) throw new Error("GIF 没有帧");

          var maxFrames = 16;
          var step = Math.max(1, Math.ceil(total / maxFrames));
          var work = new Uint8ClampedArray(W * H * 4);
          var saved = null;
          var prev = null;

          var full = document.createElement("canvas");
          full.width = W;
          full.height = H;
          var fctx = full.getContext("2d");

          var canvas = document.createElement("canvas");
          canvas.width = 160;
          canvas.height = 128;
          var ctx = canvas.getContext("2d");
          var big = document.createElement("canvas");
          big.width = 640;
          big.height = 512;
          var bctx = big.getContext("2d");
          bctx.imageSmoothingEnabled = true;
          bctx.imageSmoothingQuality = "high";

          var frames = [];
          var delaySum = 0;
          var sampled = 0;

          function clearRect(info) {
            for (var yy = info.y; yy < info.y + info.height; yy++) {
              if (yy < 0 || yy >= H) continue;
              for (var xx = info.x; xx < info.x + info.width; xx++) {
                if (xx < 0 || xx >= W) continue;
                var k = (yy * W + xx) * 4;
                work[k] = work[k + 1] = work[k + 2] = work[k + 3] = 0;
              }
            }
          }

          for (var i = 0; i < total; i++) {
            var info = reader.frameInfo(i);
            if (prev) {
              if (prev.disposal === 2) clearRect(prev);
              else if (prev.disposal === 3 && saved) work.set(saved);
            }
            if (info.disposal === 3) saved = work.slice();

            reader.decodeAndBlitFrameRGBA(i, work);
            fctx.putImageData(new ImageData(new Uint8ClampedArray(work), W, H), 0, 0);

            if (i % step === 0 && frames.length < maxFrames) {
              var scale = Math.max(640 / W, 512 / H);
              var sw = 640 / scale;
              var sh = 512 / scale;
              bctx.clearRect(0, 0, 640, 512);
              bctx.drawImage(full, (W - sw) / 2, (H - sh) / 2, sw, sh, 0, 0, 640, 512);
              ctx.clearRect(0, 0, 160, 128);
              ctx.drawImage(big, 0, 0, 160, 128);
              frames.push(packCanvasRaw(canvas));
              delaySum += (info.delay || 10) * 10 * step;
              sampled++;
            }
            prev = info;
          }

          if (!frames.length) throw new Error("没有可用帧");
          var delay = Math.round(delaySum / Math.max(1, sampled));
          delay = Math.max(30, Math.min(1000, delay));
          resolve({ frames: frames, delay: delay, total: total });
        } catch (e) {
          reject(e);
        }
      }, reject);
    });
  }

  function selectGifCapture(file) {
    return new Promise(function (resolve, reject) {
      var url = URL.createObjectURL(file);
      var img = new Image();
      var frames = [];
      var timer = null;
      var timeout = null;
      var canvas = document.createElement("canvas");
      canvas.width = 160;
      canvas.height = 128;
      var ctx = canvas.getContext("2d");
      var big = document.createElement("canvas");
      big.width = 640;
      big.height = 512;
      var bctx = big.getContext("2d");
      bctx.imageSmoothingEnabled = true;
      bctx.imageSmoothingQuality = "high";

      function cleanup() {
        if (timer) clearInterval(timer);
        if (timeout) clearTimeout(timeout);
        if (img.parentNode) img.parentNode.removeChild(img);
        URL.revokeObjectURL(url);
      }

      function finish(err) {
        cleanup();
        if (err) reject(err);
        else if (frames.length) resolve({ frames: frames, delay: 100, total: frames.length });
        else reject(new Error("没有捕获到 GIF 帧"));
      }

      function same(a, b) {
        if (a.length !== b.length) return false;
        for (var i = 0; i < a.length; i += 97) {
          if (a[i] !== b[i]) return false;
        }
        return true;
      }

      function capture() {
        if (!img.complete || !img.naturalWidth) return;
        var iw = img.naturalWidth;
        var ih = img.naturalHeight;
        var scale = Math.max(640 / iw, 512 / ih);
        var sw = 640 / scale;
        var sh = 512 / scale;
        bctx.clearRect(0, 0, 640, 512);
        bctx.drawImage(img, (iw - sw) / 2, (ih - sh) / 2, sw, sh, 0, 0, 640, 512);
        ctx.clearRect(0, 0, 160, 128);
        ctx.drawImage(big, 0, 0, 160, 128);
        var raw = packCanvasRaw(canvas);

        if (frames.length) {
          if (same(raw, frames[frames.length - 1])) return;
          if (same(raw, frames[0]) && frames.length >= 3) {
            finish();
            return;
          }
        }
        frames.push(raw);
        if (frames.length >= 16) finish();
      }

      img.onload = function () {
        // 放在视口内、几乎透明，保证浏览器继续播放 GIF。
        img.style.cssText = "position:fixed;left:0;top:0;width:160px;height:128px;" +
          "opacity:0.01;pointer-events:none;z-index:2147483647";
        document.body.appendChild(img);
        timer = setInterval(capture, 100);
        timeout = setTimeout(function () { finish(); }, 3500);
      };
      img.onerror = function () { finish(new Error("GIF 读取失败")); };
      img.src = url;
    });
  }

  async function selectGif(file) {
    stopAnimPreview();
    $("photoSize").textContent = "正在解码 GIF…";
    $("photoInfo").textContent = "浏览器逐帧解码中，请稍候";
    try {
      if (!("ImageDecoder" in window)) {
        $("photoInfo").textContent = "正在用内置 GIF 解码器拆帧…";
        try {
          var parsed = await selectGifOmggif(file);
          setAnimReady(parsed.frames, parsed.delay,
            "GIF " + parsed.total + " 帧 → 采样 " + parsed.frames.length +
            " 帧 · 160×128 无损 RGB565");
          return;
        } catch (e) {
          $("photoInfo").textContent = "内置解码失败，尝试逐帧抓取…";
          var captured = await selectGifCapture(file);
          setAnimReady(captured.frames, captured.delay,
            "GIF 逐帧抓取 " + captured.frames.length + " 帧 · 160×128 无损 RGB565");
          return;
        }
      }
      var data = await file.arrayBuffer();
      var decoder = new ImageDecoder({ data: data, type: "image/gif" });
      await decoder.tracks.ready;
      var track = decoder.tracks.selectedTrack;
      var total = track.frameCount || 0;
      if (!total) throw new Error("没有解析到 GIF 帧");

      var maxFrames = 16;
      var step = Math.max(1, Math.ceil(total / maxFrames));
      var canvas = document.createElement("canvas");
      canvas.width = 160;
      canvas.height = 128;
      var ctx = canvas.getContext("2d");
      var big = document.createElement("canvas");
      big.width = 640;
      big.height = 512;
      var bctx = big.getContext("2d");
      bctx.imageSmoothingEnabled = true;
      bctx.imageSmoothingQuality = "high";

      var frames = [];
      var delaySum = 0;
      var sampled = 0;
      for (var i = 0; i < total && frames.length < maxFrames; i += step) {
        var decoded = await decoder.decode({ frameIndex: i });
        var image = decoded.image;
        var iw = image.displayWidth || image.codedWidth;
        var ih = image.displayHeight || image.codedHeight;
        var scale = Math.max(640 / iw, 512 / ih);
        var sw = 640 / scale;
        var sh = 512 / scale;
        bctx.clearRect(0, 0, 640, 512);
        bctx.drawImage(image, (iw - sw) / 2, (ih - sh) / 2, sw, sh, 0, 0, 640, 512);
        ctx.clearRect(0, 0, 160, 128);
        ctx.drawImage(big, 0, 0, 160, 128);
        frames.push(packCanvasRaw(canvas));
        var durMs = (image.duration || track.frameDuration || 100000) / 1000;
        delaySum += durMs * step;
        sampled++;
        if (image.close) image.close();
      }
      if (!frames.length) throw new Error("没有可用的帧");
      var delay = Math.round(delaySum / sampled);
      delay = Math.max(30, Math.min(1000, delay));
      setAnimReady(frames, delay,
        "GIF " + total + " 帧 → 采样 " + frames.length + " 帧 · 160×128 无损 RGB565");
    } catch (e) {
      toast("动图解码失败：" + e.message, "error");
      clearPhotoSelection();
    }
  }

  function selectPhoto(file) {
    if (!file) return;
    if (!/^image\//.test(file.type || "")) {
      toast("请选择图片文件", "error");
      return;
    }
    if (file.size > 25 * 1024 * 1024) {
      toast("原图超过 25 MB，请先压缩", "error");
      return;
    }
    state.photoFile = file;
    if (/gif/i.test(file.type || "") || /\.gif$/i.test(file.name || "")) {
      selectGif(file);
    } else {
      selectStill(file);
    }
  }

  function renderPhotoGallery() {
    var box = $("photoGallery");
    if (!box) return;
    box.innerHTML = "";
    var data = state.photos;
    if (!data) { $("photoStorage").textContent = "正在读取…"; return; }
    $("photoStorage").textContent = "已用 " + formatBytes(data.used) + " / " +
      formatBytes(data.total) + " · " + data.count + " 个 · " +
      (data.frames || 0) + " 帧";

    if (!data.photos || !data.photos.length) {
      var empty = document.createElement("div");
      empty.className = "photo-empty";
      empty.style.gridColumn = "1/-1";
      empty.textContent = "还没有动图，先上传一个 GIF 或静态图片。";
      box.appendChild(empty);
      updatePhotoButtons();
      return;
    }

    data.photos.forEach(function (p) {
      var item = document.createElement("div");
      item.className = "photo-thumb";
      var img = document.createElement("canvas");
      img.width = 160;
      img.height = 128;
      fetch("/api/photo?t=" + Date.now())
        .then(function (r) { return r.arrayBuffer(); })
        .then(function (buf) {
          var a = new Uint8Array(buf);
          if (a.length >= 12 + 160 * 128 * 2 && a[0] === 65 && a[1] === 78 &&
              a[2] === 73 && a[3] === 77) {
            drawRawPhoto(img, a.slice(12, 12 + 160 * 128 * 2).buffer);
          }
        })
        .catch(function () {});
      var del = document.createElement("button");
      del.type = "button";
      del.textContent = "×";
      del.title = "删除";
      del.addEventListener("click", function () { deletePhoto(); });
      var cap = document.createElement("small");
      cap.textContent = (p.frames || 1) + " 帧 · " + formatBytes(p.size);
      item.appendChild(img);
      item.appendChild(del);
      item.appendChild(cap);
      box.appendChild(item);
    });
    updatePhotoButtons();
  }

  async function loadPhotos() {
    try {
      state.photos = await api("/api/photos", null, 6000);
      renderPhotoGallery();
    } catch (e) {
      $("photoStorage").textContent = "读取动图失败";
    }
  }

  async function uploadPhoto() {
    if (state.uploadingPhoto || !state.photoBlob) return;
    if (state.photoBlob.size > 700000) {
      toast("动图超过 700 KB，请减少帧数或尺寸", "error");
      return;
    }
    var fd = new FormData();
    fd.append("photo", state.photoBlob, "photo.anim");
    state.uploadingPhoto = true;
    updatePhotoButtons();
    try {
      var resp = await fetch("/api/photo/upload", { method: "POST", body: fd });
      var data = await resp.json();
      if (!resp.ok || !data.ok) throw new Error(data.error || "上传失败");
      toast("已上传 " + (data.frames || 1) + " 帧，约 " + formatBytes(data.size), "ok");
      clearPhotoSelection();
      await loadPhotos();
    } catch (e) {
      toast("上传失败：" + e.message, "error");
    } finally {
      state.uploadingPhoto = false;
      updatePhotoButtons();
    }
  }

  async function deletePhoto() {
    if (!window.confirm("删除当前动图/照片？")) return;
    try {
      await postForm("/api/photo/delete", {});
      toast("已删除", "ok");
      await loadPhotos();
    } catch (e) {
      toast("删除失败：" + e.message, "error");
    }
  }

  function fillStatus(s) {
    state.status = s;
    var online = s.mode === "sta" && s.wifi_connected;
    var ap = s.mode === "ap";
    setStatus(online ? "online" : (ap ? "ap" : (s.wifi_connected ? "online" : "off")),
      online ? "设备在线" : (ap ? "配网模式" : (s.wifi_connected ? "已连接" : "离线")));
    $("ipPill").textContent = s.ip || "--";
    $("ipPill").title = s.ip || "";
    $("statSsid").textContent = s.ssid || (ap ? "配网热点" : "未连接");
    $("statRssi").textContent = s.wifi_connected ? rssiText(s.rssi) : "--";
    $("statUptime").textContent = fmtUptime(s.uptime);
    $("statHeap").textContent = Math.round((s.heap || 0) / 1024) + " KB";
    $("deviceDate").textContent = s.time_valid ? s.date : "等待 NTP 对时";
    setClockBase(s.time);
    tickClock();
    $("infoChip").textContent = s.chip || "ESP32-C3";
    $("infoMac").textContent = s.mac || "--";
    $("infoAp").textContent = s.ap_ssid || "--";
    $("infoMdns").textContent = s.mdns || "esp32clock.local";
    fillWeather(s.weather);

    var banner = $("apBanner");
    if (ap) {
      $("apBannerSsid").textContent = s.ap_ssid || "ESP32-Clock";
      banner.hidden = false;
    } else {
      banner.hidden = true;
    }
  }

  async function loadConfig() {
    try {
      fillConfig(await api("/api/config", null, 6000));
    } catch (e) {
      $("saveHint").textContent = "读取配置失败";
      toast("读取配置失败：" + e.message, "error");
    }
  }

  async function loadStatus(silent) {
    try {
      fillStatus(await api("/api/status", null, 5000));
    } catch (e) {
      if (!silent) setStatus("off", "连接中断");
    }
  }

  function setPassDisabled(disabled) {
    $("pass").disabled = disabled;
    $("eyeBtn").disabled = disabled;
    $("pass").placeholder = disabled ? "开放网络不需要密码" : ($("pass").value ? "" : (state.cfg && state.cfg.pass_set ? "已保存密码，留空则不修改" : "请输入 Wi-Fi 密码"));
  }

  function barsEl(rssi) {
    var level = rssi >= -55 ? 4 : rssi >= -67 ? 3 : rssi >= -78 ? 2 : 1;
    var wrap = document.createElement("span");
    wrap.className = "bars";
    for (var i = 1; i <= 4; i++) {
      var bar = document.createElement("i");
      bar.style.height = (4 + i * 2) + "px";
      if (i <= level) bar.className = "on";
      wrap.appendChild(bar);
    }
    return wrap;
  }

  function renderScan(list) {
    var panel = $("scanPanel");
    panel.hidden = false;
    panel.innerHTML = "";
    if (!list || !list.length) {
      var empty = document.createElement("div");
      empty.className = "scan-empty";
      empty.textContent = "没有找到 Wi-Fi，请靠近路由器后重试";
      panel.appendChild(empty);
      return;
    }
    list.sort(function (a, b) { return b.rssi - a.rssi; });
    list.forEach(function (net) {
      var btn = document.createElement("button");
      btn.type = "button";
      btn.className = "scan-item";
      btn.title = net.ssid + " · " + net.rssi + " dBm";
      var name = document.createElement("span");
      name.className = "scan-ssid";
      name.textContent = net.ssid;
      var meta = document.createElement("span");
      meta.className = "scan-meta";
      if (net.enc) {
        var lock = document.createElementNS("http://www.w3.org/2000/svg", "svg");
        lock.setAttribute("viewBox", "0 0 24 24");
        lock.setAttribute("fill", "none");
        lock.setAttribute("stroke", "currentColor");
        lock.setAttribute("stroke-width", "1.8");
        lock.innerHTML = '<rect x="5" y="10" width="14" height="10" rx="2"/><path d="M8 10V7a4 4 0 0 1 8 0v3"/>';
        meta.appendChild(lock);
      }
      meta.appendChild(barsEl(net.rssi));
      btn.appendChild(name);
      btn.appendChild(meta);
      btn.addEventListener("click", function () {
        $("ssid").value = net.ssid;
        panel.hidden = true;
        markDirty();
        if (!$("openNet").checked) $("pass").focus();
      });
      panel.appendChild(btn);
    });
  }

  async function scan() {
    var btn = $("scanBtn");
    btn.disabled = true;
    var old = btn.innerHTML;
    btn.innerHTML = '<span class="spinner" style="display:block;border-color:rgba(94,234,212,.2);border-top-color:#5eead4"></span>';
    try {
      var data = await api("/api/scan", null, 20000);
      renderScan(data.networks);
    } catch (e) {
      toast("扫描失败：" + e.message, "error");
    } finally {
      btn.disabled = false;
      btn.innerHTML = old;
    }
  }

  function showReboot(opts) {
    opts = opts || {};
    $("rebootOverlay").hidden = false;
    $("rebootTitle").textContent = opts.title || "正在保存并重启";
    $("rebootText").textContent = opts.text || "设备会在几秒后重新上线，请不要断开电源。";
    var hint = $("rebootHint");
    if (opts.hint) { hint.hidden = false; hint.textContent = opts.hint; }
    else hint.hidden = true;
    if (state.poll) { clearInterval(state.poll); state.poll = null; }
    var started = Date.now();
    var wasAp = state.status && state.status.mode === "ap";
    state.poll = setInterval(async function () {
      try {
        var s = await api("/api/status", null, 2500);
        clearInterval(state.poll); state.poll = null;
        $("rebootTitle").textContent = "设备已重新上线";
        $("rebootText").textContent = "正在刷新配置页面…";
        setTimeout(function () { location.reload(); }, 1200);
      } catch (e) {
        var sec = Math.floor((Date.now() - started) / 1000);
        if (wasAp && sec > 12) {
          $("rebootHint").hidden = false;
          $("rebootHint").textContent = "设备已切换到家中 Wi-Fi。请连接该 Wi-Fi，然后访问 http://esp32clock.local 或路由器里查到的设备 IP。";
        }
      }
    }, 2200);
  }

  function askConfirm(title, text, okLabel) {
    return new Promise(function (resolve) {
      var dlg = $("confirmDlg");
      if (!dlg || typeof dlg.showModal !== "function") {
        resolve(window.confirm(title + "\n\n" + text));
        return;
      }
      $("dlgTitle").textContent = title;
      $("dlgText").textContent = text;
      $("dlgOk").textContent = okLabel || "确定";
      dlg.returnValue = "cancel";
      var onClose = function () {
        dlg.removeEventListener("close", onClose);
        resolve(dlg.returnValue === "ok");
      };
      dlg.addEventListener("close", onClose);
      dlg.showModal();
    });
  }

  async function save() {
    if (state.saving) return;
    var ssid = $("ssid").value.trim();
    if (!ssid) {
      toast("请填写 Wi-Fi 名称", "error");
      $("ssid").focus();
      return;
    }
    var open = $("openNet").checked;
    var pass = $("pass").value;
    if (!open && !pass && !(state.cfg && state.cfg.pass_set)) {
      toast("请填写 Wi-Fi 密码，或勾选“开放网络”", "error");
      $("pass").focus();
      return;
    }
    if (!open && state.cfg && ssid !== state.cfg.ssid && !pass && state.cfg.pass_set) {
      toast("更换 Wi-Fi 后请重新输入密码", "error");
      $("pass").focus();
      return;
    }
    var params = {
      ssid: ssid,
      pass: open ? "" : pass,
      open: open ? "1" : "0",
      tz: $("tz").value.trim() || "CST-8",
      ntp1: $("ntp1").value.trim(),
      ntp2: $("ntp2").value.trim(),
      brightness: $("brightness").value,
      twelve_hour: $("h12").checked ? "1" : "0",
      show_seconds: $("showSeconds").checked ? "1" : "0",
      invert: $("invert").checked ? "1" : "0",
      color_order: $("colorOrder").value,
      rotation: $("rotation").value,
      offset_x: $("offsetX").value,
      offset_y: $("offsetY").value,
      refresh_mode: $("refreshMode").value,
      weather_enabled: $("weatherEnabled").checked ? "1" : "0",
      weather_key: $("weatherKey").value.trim(),
      weather_location: $("weatherLocation").value.trim() || "beijing",
      weather_update: $("weatherUpdate").value,
      weather_page: $("weatherPage").value,
      weather_layout: $("weatherLayout").value,
      photo_page_sec: $("photoPageSec").value,
      photo_time_on: $("photoTimeOn").checked ? "1" : "0",
      photo_time_style: $("photoTimeStyle").value,
      photo_time_color: $("photoTimeColor").value,
      photo_colon_style: $("photoColon").value,
      photo_grad_on: $("photoGradOn").checked ? "1" : "0",
      photo_grad_color: $("photoGradColor").value,
      photo_grad_opa: $("photoGradOpa").value,
      photo_grad_height: $("photoGradHeight").value,
      page_show_clock: $("pageShowClock").checked ? "1" : "0",
      page_show_weather: $("pageShowWeather").checked ? "1" : "0",
      page_show_forecast: $("pageShowForecast").checked ? "1" : "0",
      page_show_photo: $("pageShowPhoto").checked ? "1" : "0",
      page_order_clock: $("pageOrderClock").value,
      page_order_weather: $("pageOrderWeather").value,
      page_order_forecast: $("pageOrderForecast").value,
      page_order_photo: $("pageOrderPhoto").value,
      resident_page: $("residentPage").value
    };
    var wasAp = state.status && state.status.mode === "ap";
    setSaving(true);
    try {
      var data = await postForm("/api/config", params);
      markClean();
      toast(data.message || "保存成功，设备正在重启");
      showReboot({
        title: "保存成功，正在重启",
        text: wasAp ? "设备正在连接新的 Wi-Fi。" : "设备会在几秒后重新上线，请不要断开电源。",
        hint: wasAp ? "如果页面没有自动恢复，请连接家中 Wi-Fi，再访问 http://esp32clock.local。" : ""
      });
    } catch (e) {
      toast("保存失败：" + e.message, "error");
      setSaving(false);
    }
  }

  function bindDirtyListeners() {
    ["ssid", "pass", "tz", "ntp1", "ntp2", "brightness", "rotation", "colorOrder", "offsetX", "offsetY", "invert", "showSeconds", "h12", "h24", "openNet", "weatherEnabled", "weatherKey", "weatherLocation", "weatherUpdate", "weatherPage", "weatherLayout", "photoPageSec", "photoTimeOn", "photoTimeStyle", "photoTimeColor", "photoTimeHex", "photoColon", "photoGradOn", "photoGradColor", "photoGradHex", "photoGradOpa", "photoGradHeight", "pageShowClock", "pageShowWeather", "pageShowForecast", "pageShowPhoto", "pageOrderClock", "pageOrderWeather", "pageOrderForecast", "pageOrderPhoto", "residentPage"].forEach(function (id) {
      var el = $(id);
      if (!el) return;
      el.addEventListener("input", markDirty);
      el.addEventListener("change", markDirty);
    });

    function pair(colorId, hexId) {
      var c = $(colorId), h = $(hexId);
      if (!c || !h) return;
      c.addEventListener("input", function () {
        h.value = c.value.toUpperCase();
      });
      h.addEventListener("input", function () {
        var v = h.value.trim();
        if (/^#[0-9a-fA-F]{6}$/.test(v)) c.value = v;
      });
    }
    pair("photoTimeColor", "photoTimeHex");
    pair("photoGradColor", "photoGradHex");

    var opa = $("photoGradOpa");
    if (opa) {
      opa.addEventListener("input", function () {
        $("photoGradOpaVal").textContent = opa.value + "%";
      });
    }
  }

  async function refreshWeather() {
    var btn = $("weatherRefresh");
    if (btn.disabled) return;
    btn.disabled = true;
    var old = btn.textContent;
    btn.textContent = "正在更新…";
    try {
      await postForm("/api/weather/refresh", {});
      toast("已请求更新天气，几秒后刷新状态", "ok");
      setTimeout(function () { loadStatus(true); }, 5000);
    } catch (e) {
      toast("更新失败：" + e.message, "error");
    } finally {
      btn.disabled = false;
      btn.textContent = old;
    }
  }

  function bind() {
    $("scanBtn").addEventListener("click", scan);
    $("saveBtn").addEventListener("click", save);
    $("weatherRefresh").addEventListener("click", refreshWeather);

    $("photoDrop").addEventListener("click", function () { $("photoFile").click(); });
    $("photoFile").addEventListener("click", function (e) { e.stopPropagation(); });
    $("photoFile").addEventListener("change", function () { selectPhoto(this.files[0]); });
    ["dragenter", "dragover"].forEach(function (ev) {
      $("photoDrop").addEventListener(ev, function (e) {
        e.preventDefault();
        this.classList.add("over");
      });
    });
    ["dragleave", "drop"].forEach(function (ev) {
      $("photoDrop").addEventListener(ev, function (e) {
        e.preventDefault();
        this.classList.remove("over");
      });
    });
    $("photoDrop").addEventListener("drop", function (e) {
      var files = e.dataTransfer && e.dataTransfer.files;
      if (files && files.length) selectPhoto(files[0]);
    });
    $("photoUpload").addEventListener("click", uploadPhoto);
    $("photoClear").addEventListener("click", clearPhotoSelection);

    $("eyeBtn").addEventListener("click", function () {
      var p = $("pass");
      p.type = p.type === "password" ? "text" : "password";
      this.title = p.type === "password" ? "显示密码" : "隐藏密码";
    });

    $("openNet").addEventListener("change", function () {
      setPassDisabled(this.checked);
      if (this.checked) $("pass").value = "";
      markDirty();
    });

    $("tzPreset").addEventListener("change", function () {
      if (this.value !== "custom") {
        $("tz").value = this.value;
      } else {
        $("tz").focus();
      }
      markDirty();
    });

    $("tz").addEventListener("input", function () {
      var val = this.value.trim();
      var preset = $("tzPreset");
      var found = false;
      for (var i = 0; i < preset.options.length; i++) {
        if (preset.options[i].value === val) { preset.value = val; found = true; break; }
      }
      if (!found) preset.value = "custom";
    });

    $("brightness").addEventListener("input", function () {
      var v = +this.value;
      $("brightnessVal").textContent = v + "%";
      this.style.setProperty("--fill", v + "%");
      clearTimeout(this._previewTimer);
      var self = this;
      this._previewTimer = setTimeout(function () {
        fetch("/api/brightness?value=" + encodeURIComponent(self.value), { cache: "no-store" }).catch(function () {});
      }, 220);
      markDirty();
    });

    $("restartBtn").addEventListener("click", async function () {
      var ok = await askConfirm("重启设备？", "重启期间时钟短暂离线，配置不会丢失。", "重启");
      if (!ok) return;
      try {
        await postForm("/api/restart", {});
        showReboot({ title: "设备正在重启", text: "大约需要 5 秒，页面会自动刷新。", hint: "" });
      } catch (e) {
        toast("重启失败：" + e.message, "error");
      }
    });

    $("factoryBtn").addEventListener("click", async function () {
      var ok = await askConfirm("恢复出厂设置？", "将清除 Wi-Fi、时区和显示设置。设备重启后会开启配网热点，需要重新连接。", "恢复出厂");
      if (!ok) return;
      try {
        await postForm("/api/factory", {});
        showReboot({
          title: "已恢复出厂设置",
          text: "设备正在重启并进入配网模式。",
          hint: "请连接热点 ESP32-Clock-XXXX，然后访问 http://192.168.4.1。"
        });
      } catch (e) {
        toast("恢复失败：" + e.message, "error");
      }
    });

    window.addEventListener("beforeunload", function (e) {
      if (state.dirty && !state.saving) {
        e.preventDefault();
        e.returnValue = "";
      }
    });
  }

  async function boot() {
    setStatus("", "正在连接");
    bind();
    await loadConfig();
    await loadPhotos();
    bindDirtyListeners();
    await loadStatus(false);
    setInterval(function () { loadStatus(true); }, 5000);
    setInterval(tickClock, 1000);
    tickClock();
  }

  document.addEventListener("DOMContentLoaded", boot, { once: true });
})();
</script>
</body>
</html>
)HTML";
