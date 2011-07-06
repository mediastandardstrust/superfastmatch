{{>HEADER}}
	<h1>Status</h1>
	{{>POSTING_STATS}}
	<h2>Total Memory Usage: {{MEMORY}} GB</h2>
	{{#DB}}
	<h2>{{NAME}}</h2>
	<pre>{{STATS}}</pre>
	{{/DB}}		
	<h2>Memory:</h2>
	<pre>{{MEMORY_STATS}}</pre>
{{>FOOTER}}