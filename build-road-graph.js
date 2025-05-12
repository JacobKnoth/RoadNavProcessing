const { MongoClient } = require('mongodb');

async function run() {
  console.log('Running script');
  const uri = 'mongodb://localhost:27017';
  const client = new MongoClient(uri);
  await client.connect();

  const db = client.db('osm');
  const collection = db.collection('all_ways');

  // Create a map of node coords to connected way IDs
  const nodeMap = {};

  const cursor = collection.find().limit(1000);
  console.log('Query initialized');

  while (await cursor.hasNext()) {
    const doc = await cursor.next();
    const coords = doc.geometry?.coordinates;

    if (!coords || coords.length < 2) continue; // Skip if invalid geometry

    const id = doc._id.toString(); // use _id as way ID
    const start = coords[0];
    const end = coords[coords.length - 1];

    const formatCoord = ([lon, lat]) => `${lat.toFixed(7)},${lon.toFixed(7)}`;

    const startKey = formatCoord(start);
    const endKey = formatCoord(end);

    if (!nodeMap[startKey]) nodeMap[startKey] = [];
    if (!nodeMap[endKey]) nodeMap[endKey] = [];

    nodeMap[startKey].push(id);
    nodeMap[endKey].push(id);
  }

  await client.close();

  // Print connections (nodes with multiple ways)
  const intersections = Object.entries(nodeMap).filter(([_, ways]) => ways.length > 1);
  console.log(`Found ${intersections.length} intersections`);
  console.log(intersections.slice(0, 2)); // Show a sample

  // save as JSON
   const fs = require('fs');
   fs.writeFileSync(
    'road-node-map.json',
    JSON.stringify(nodeMap, null, 2)
  );
  console.log('Saved road-node-map.json');
}

run().catch(console.error);
