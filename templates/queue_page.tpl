{{>HEADER}}
	<h1>Queue</h1>
	<table>
		<caption>{{>PAGING}}</caption>
		<thead>
			<th>Status</th>
			<th>ID</th>
			<th>Priority</th>
			<th>Action</th>
			<th>Doc Type</th>
			<th>Doc ID</th>
		</thead>
		<tbody>
			{{#COMMAND}}
			<tr>
				<td>{{STATUS}}</td>
				<td>{{ID}}</td>
				<td>{{PRIORITY}}</td>
				<td>{{ACTION}}</td>
				<td><a href="/document/{{DOC_TYPE}}/">{{DOC_TYPE}}</a></td>
				<td><a href="/document/{{DOC_TYPE}}/{{DOC_ID}}/">{{DOC_ID}}</a></td>
			</tr>
			{{/COMMAND}}
		</tbody>
	</table>
{{>FOOTER}}