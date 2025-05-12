#!/usr/bin/env python3

# ZVM Benchmarks project
# Copyright 2021 The EVM Benchmarks Authors.
# SPDX-License-Identifier: Apache-2.0

import sys

BASE_FEE = 1
GAS_LIMIT = 10 ** 9

ORIGIN = 'Z83625b2f4001029facfc3355971ebc94c28c56c3d2b66ea9' # address
ORIGIN_SEED = '0x41f6e321b31e72173f8ff2e292359e1862f24fba42fe6f97efaf641980eff29862f24fba42fe6f97efaf641980eff298'
COINBASE = ORIGIN
RANDOM = '0xa94f5374fce5edbc8e2a8697c15331677e6ebf0b' # hash

NETWORK = 'Shanghai'


# Decode DRY text.
# The DRY text can have patterns like `(` n `*` s `)` which are expanded during
# decoding by repeating the s string n times.
def dry_decode(d: str) -> str:
    parts = []
    pos = 0
    while True:
        try:
            b = d.index('(', pos)
            e = d.index(')', b + 1)
        except ValueError:
            break  # No more "(...)"

        try:
            x = d.index('*', b + 1, e)
            n = int(d[b + 1:x])
            parts.append(d[pos:b])  # All before "(".
            parts.append(n * d[x + 1:e])  # Repeated content.
        except ValueError:
            parts.append(d[pos:e + 1])  # If "*" missing, append "...(...)"
        pos = e + 1

    parts.append(d[pos:])  # Append what's left.

    return ''.join(parts)


# Creates State Test source file (filler) out of a benchmark source file.
# This mostly fills the YAML structure with default values required by
# the State Test specification and generates "expect" section.
# https://ethereum-tests.readthedocs.io/en/latest/test_filler/state_filler.html
def build_source(src_file, out_file):
    import yaml
    import pathlib

    test_name = pathlib.Path(src_file.name).stem  # State Test convention: file name matches dict key.
    document = yaml.load(src_file, Loader=yaml.SafeLoader)
    test = document[test_name]

    env = test.setdefault('env', {})
    env.setdefault('currentBaseFee', BASE_FEE)
    env.setdefault('currentCoinbase', COINBASE)
    env.setdefault('currentGasLimit', GAS_LIMIT)
    env.setdefault('currentRandom', RANDOM)
    env.setdefault('currentNumber', 1)
    env.setdefault('currentTimestamp', 1638453897)
    env.setdefault('withdrawals', [])

    pre = test['pre']
    pre.setdefault(ORIGIN, {}).setdefault('balance', GAS_LIMIT * BASE_FEE)
    for account in pre.values():
        account.setdefault('balance', 0)
        account.setdefault('code', '')
        account.setdefault('nonce', 0)
        account.setdefault('storage', {})

    tx = test['transaction']
    tx.setdefault('gasLimit', [GAS_LIMIT])
    tx.setdefault('value', [0])
    tx.setdefault('nonce', 0)
    tx.setdefault('maxFeePerGas', BASE_FEE)
    tx.setdefault('maxPriorityFeePerGas', 0)
    tx.setdefault('seed', ORIGIN_SEED)

    expect = []
    for i, data in enumerate(tx['data']):
        assert data.startswith(':label')
        label_end = data.index(' ', len(':label '))
        label = data[0:label_end]
        expect.append({
            'indexes': {'data': label, 'gas': -1, 'value': -1},
            'network': [NETWORK],
            'result': {}
        })
        tx['data'][i] = dry_decode(data)
    assert 'expect' not in test
    test['expect'] = expect

    yaml.dump(document, out_file, sort_keys=False)


if __name__ == '__main__':
    import argparse

    cli = argparse.ArgumentParser()
    commands = cli.add_subparsers(dest='command', help='Commands')
    build_source_cmd = commands.add_parser(
        'build-source',
        help='Creates State Transition Test Source out of a benchmark source file',
    )
    build_source_cmd.add_argument('file', type=argparse.FileType('r'), default=sys.stdin, help='Benchmark source file')
    build_source_cmd.add_argument('-o', dest='out', type=argparse.FileType('w'), default=sys.stdout, help='Output file')

    args = cli.parse_args()
    if args.command == 'build-source':
        build_source(args.file, args.out)
