//
//  ViewController.m
//  OpusFileDecoderDemo
//
//  Created by Drops on 2024/6/23.
//

#import "ViewController.h"
#import "OpusDecoder/OpusFileDecoder.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    NSString *testDir = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject stringByAppendingPathComponent:@"test-opus"];
    [self test:testDir];
}

- (void)test:(NSString *)testDir {
    NSArray *subPaths = [NSFileManager.defaultManager subpathsAtPath:testDir];
    NSLog(@"-------------------begin----------------");
    for (NSString *path in subPaths) {
        BOOL isDir = NO;
        NSString *opusPath = [testDir stringByAppendingPathComponent:path];
        if ([NSFileManager.defaultManager fileExistsAtPath:opusPath isDirectory:&isDir] && !isDir && [opusPath hasSuffix:@"opus"]) {
            NSString *wavPath = [opusPath stringByAppendingString:@".wav"];
            int ret = opus2wav(opusPath.UTF8String, wavPath.UTF8String);
            NSLog(@"ret:%d %@", ret, wavPath);
        }
    }
    
    NSLog(@"-------------------end----------------");
}

@end
