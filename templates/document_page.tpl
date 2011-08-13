{{>HEADER}}
	<h1>Document</h1>
	<h2>Meta</h2>
	<table>
		<thead>
			<th>
				Key
			</th>
			<th>
				Value
			</th>
		</thead>
		<tbody>
			{{#META}}
			<tr>
				<td class="line">{{KEY}}</td>
				<td class="line">{{VALUE}}</td>
			</tr>
			{{/META}}
		</tbody>
	</table>
	<h2>Associations</h2>
{{>ASSOCIATION}}
	<h2>Text</h2>
	<pre>{{TEXT}}</pre>
	
{{>FOOTER}}