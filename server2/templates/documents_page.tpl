{{>HEADER}}
	<h1>Documents</h1>
	<table>
		<caption>{{>PAGING}}</caption>
		<thead>
			<th>
				Document Type
			</th>
			<th>
				Document ID
			</th>
			<th>
				Title
			</th>
		</thead>
		<tbody>
			{{#DOCUMENT}}
			<tr>
				<td class="line"><a href="/document/{{DOC_TYPE}}/">{{DOC_TYPE}}</a></td>
				<td class="line"><a href="/document/{{DOC_TYPE}}/{{DOC_ID}}/">{{DOC_ID}}</a></td>
				<td class="line"><a href="/document/{{DOC_TYPE}}/{{DOC_ID}}/">{{DOC_TITLE}}</a></td>
			</tr>
			{{/DOCUMENT}}
		</tbody>
	</table>
{{>FOOTER}}