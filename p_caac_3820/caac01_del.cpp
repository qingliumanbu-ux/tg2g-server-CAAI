/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhl
Version:    1.0
Date:     2016-03-10
Description: 成本中心删除
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"


// service入口
BM2F_ENTERACE(caac01_del)

int f_caac01_del(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

    /* 程序内部变量 */
	int   doFlag = 0;
    int   fetchRowCount = 0;
	int   outBlockRow=0;
	

    /* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
	CModel tcaac01("TCAAC01");
	CModel tcaac04("TCAAC04");

	// 数据库SQL操作字符串
	CString  sql("");


    try
    { 
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
        //获得传入的数据行数
		int count = bcls_rec->Tables[0].Rows.get_Count();
		
		for (int i = 0; i <  count ; i++ )
		{
            //重置头文件,获得头文件中的默认值
			tcaac01.Reset();

			//取得一行数据， MergeFrom方法将获得bcls_rec指定表的指定行的数据
			tcaac01.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			if (tcaac01["COST_CENTER"].ToString().Trim() == "")
			{
				sprintf(s.msg,"成本中心为空!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

            //删除一条记录
            tcaac01.Delete("COST_CENTER,DEPT_CODE");
			//tcaac03.COST_CENTER = tcaac01.COST_CENTER;
			//tcaac03.Delete("COST_CENTER"); //成本收集规则表
			tcaac04["COST_CENTER"] = tcaac01["COST_CENTER"];
			tcaac04.Delete("COST_CENTER"); //成本中心,中心对应关系表
			//tcaac05.COST_CENTER = tcaac01.COST_CENTER;
			//tcaac05.Delete("COST_CENTER"); //标准单耗
			//tcaac07.COST_CENTER = tcaac01.COST_CENTER;
			//tcaac07.Delete("COST_CENTER"); //费用分摊规则表
		}  
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,"数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。" /* _RES("GCRSS0000006")*//*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sql + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		EDLog(1,1, "[%s]", s.sysmsg);
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}
