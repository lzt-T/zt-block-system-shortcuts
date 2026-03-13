const { disableAll, enableAll, KeyManager } = require('../index');

console.log('=== zt-block-system-shortcuts basic test ===\n');

console.log('Testing function exports...');
console.log('  disableAll:', typeof disableAll === 'function' ? 'OK' : 'FAIL');
console.log('  enableAll:', typeof enableAll === 'function' ? 'OK' : 'FAIL');
console.log('  KeyManager:', typeof KeyManager === 'function' ? 'OK' : 'FAIL');

console.log('\nTesting KeyManager class...');
const km = new KeyManager();
console.log('  instance created:', km instanceof KeyManager ? 'OK' : 'FAIL');
console.log('  disableAll method:', typeof km.disableAll === 'function' ? 'OK' : 'FAIL');
console.log('  enableAll method:', typeof km.enableAll === 'function' ? 'OK' : 'FAIL');

console.log('\nTesting disableAll/enableAll toggle...');
console.log('  Calling disableAll()...');
const disableResult = disableAll();
console.log('  disableAll returned:', disableResult);

console.log('  Waiting 2 seconds...');
setTimeout(() => {
  console.log('  Calling enableAll()...');
  const enableResult = enableAll();
  console.log('  enableAll returned:', enableResult);
  console.log('\n=== Test completed ===');
}, 2000);
