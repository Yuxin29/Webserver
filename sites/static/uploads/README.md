# Upload Directory

This directory is for testing file uploads using POST method.

## Testing Upload:
```bash
# Upload a file
curl -X POST -F "file=@myfile.txt" http://localhost:8080/uploads/myfile.txt

# List uploaded files
curl http://localhost:8080/uploads

# Delete a file
curl -X DELETE http://localhost:8080/uploads/myfile.txt
```
