# Project Overview #
DirectShow subtitle renderer using DirectWrite and Direct2D.

## SRT Support ##
Because no official format specification exists for SRT and different players support different features added into SRT (most notably by VSFilter's hackish way of treating SRT subtitles as a special case of ASS), we will support a very strict subset of the potential features of the format. Specifically, we support:
```
	<b></b>			HTML-Bold
	<u></u>			HTML-Underline
	<i></i>			HTML-Italic
	<s></s>			HTML-Strikethrough
```
and that is it. All other tags will simply be displayed as if it were normal text. SRT really should just be a plain-text format; styling and positioning should be done through ASS.